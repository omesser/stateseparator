<?php
$pageTitle = 'Results - State Separator';
$pageDescription = 'State Separator analysis results.';
$currentPage = 'home';

$separate = $_POST['separate'] ?? null;
$randomize = $_POST['randomize'] ?? null;

// Example title persistence
$exampleTitle = $_POST['exampleTitle'] ?? "";
$exampleDesc = $_POST['exampleDesc'] ?? "";
$isExample = !empty($exampleTitle);
if ($isExample) {
    $currentPage = 'examples';
}

if ($separate || $randomize) {
    $particleSizes = $_POST['particleSizes'] ?? "2 2";
    $targetDistance = $_POST['targetDistance'] ?? "";
    $minProbForState = $_POST['minProbForState'] ?? "";
    $targetNumberOfStates = $_POST['targetNumberOfStates'] ?? "";
    $precision = $_POST['precision'] ?? "3";
    $accuracyBoost = $_POST['accuracyBoost'] ?? "0";
} else {
    $particleSizes = "2   2";
    $targetDistance = "";
    $minProbForState = "";
    $targetNumberOfStates = "";
    $precision = "3";
    $accuracyBoost = "0";
}

$matrixInput = "";
$outputResult = "";

const INPUT_INT_LIST = '/^\s*\d+(\s+\d+)*\s*$/';
const INPUT_INT = '/^\s*\d+\s*$/';
const INPUT_INT_OPTIONAL = '/^\s*(\d+)?\s*$/';
const INPUT_DECIMAL_OPTIONAL = '/^\s*([+-]?(\d+\.?\d*|\.\d+)([eE][+-]?\d+)?)?\s*$/';
const INPUT_MATRIX_CHARS = '/^[0-9.eE+\-ij \t\r\n]*$/';

function rejectInput(string $message): void
{
    http_response_code(400);
    header('Content-Type: text/plain; charset=utf-8');
    echo "Invalid input: $message\n";
    exit;
}

// $rules maps label => [value, regex]; the first mismatch ends the request with HTTP 400.
function validateInputs(array $rules): void
{
    foreach ($rules as $label => [$value, $pattern]) {
        if (!is_string($value) || !preg_match($pattern, $value)) {
            rejectInput("$label has characters or a format that is not allowed.");
        }
    }
}

// Builds argv from the same ordered field list that validateInputs() checked.
function fieldArgv(string $binary, array $fields): array
{
    return array_merge([$binary], array_column(array_values($fields), 0));
}

// Runs a binary from this directory with an argv array, so no shell parses the input.
// Returns stdout lines with trailing whitespace stripped, the way exec() did.
function runBinary(array $argv, ?int &$exitCode): array
{
    $stdout = '';
    $proc = proc_open($argv, [1 => ['pipe', 'w'], 2 => fopen('php://stderr', 'w')], $pipes, __DIR__);
    if (is_resource($proc)) {
        $stdout = stream_get_contents($pipes[1]);
        fclose($pipes[1]);
        $exitCode = proc_close($proc);
    } else {
        $exitCode = -1;
    }
    $lines = explode("\n", $stdout);
    if (end($lines) === '') {
        array_pop($lines);
    }
    return array_map(fn($line) => rtrim($line, " \t\n\r\v\f"), $lines);
}

if ($separate) {
    $matrix = $_POST['matrix'] ?? "";
    if (!is_string($matrix)) {
        rejectInput('Matrix must be text.');
    }
    $outputMarkers = array('The system is', 'ERROR:', 'The Eigenvalues');
    $cleanMatrix = $matrix;
    foreach ($outputMarkers as $marker) {
        $pos = strpos($cleanMatrix, $marker);
        if ($pos !== false) {
            $cleanMatrix = substr($cleanMatrix, 0, $pos);
        }
    }
    $matrixInput = trim($cleanMatrix);
    $fields = [
        'System (particle sizes)' => [$particleSizes, INPUT_INT_LIST],
        'Matrix' => [$matrixInput, INPUT_MATRIX_CHARS],
        'Target distance' => [$targetDistance, INPUT_DECIMAL_OPTIONAL],
        'Min probability' => [$minProbForState, INPUT_DECIMAL_OPTIONAL],
        'Target states' => [$targetNumberOfStates, INPUT_INT_OPTIONAL],
        'Precision' => [$precision, INPUT_INT],
        'Accuracy boost' => [$accuracyBoost, INPUT_INT],
    ];
    validateInputs($fields);
    $matrixRows = substr_count($matrixInput, "\n") + 1;
    error_log("[NSeparator] Executing: particleSizes=$particleSizes, matrixRows=$matrixRows, precision=$precision, accuracyBoost=$accuracyBoost");
    $separatorOutput = runBinary(fieldArgv('./NSeparator', $fields), $returnValue);
    error_log("[NSeparator] Completed: exitCode=$returnValue");
    $outputResult = implode("\n", $separatorOutput);
} elseif ($randomize) {
    $fields = [
        'System (particle sizes)' => [$particleSizes, INPUT_INT_LIST],
        'Precision' => [$precision, INPUT_INT],
    ];
    validateInputs($fields);
    error_log("[NRandomizer] Executing: particleSizes=$particleSizes, precision=$precision");
    $randomizerOutput = runBinary(fieldArgv('./NRandomizer', $fields), $returnValue);
    error_log("[NRandomizer] Completed: exitCode=$returnValue");
    $matrixInput = implode("\n", $randomizerOutput);
    $outputResult = "Random matrix generated. Click 'Separate' to analyze.";
}

include '_header.php';
include '_nav.php';
?>

    <main>
      <div class="container">
        <form method="post" action="./calculate.php">
          <div class="results-grid">
            <div class="matrix-card">
<?php if ($exampleTitle) : ?>
              <div class="example-header">
                <h1><?php echo htmlspecialchars($exampleTitle); ?></h1>
                <p><?php echo htmlspecialchars($exampleDesc); ?></p>
              </div>
              <input type="hidden" name="exampleTitle" value="<?php echo htmlspecialchars($exampleTitle); ?>" />
              <input type="hidden" name="exampleDesc" value="<?php echo htmlspecialchars($exampleDesc); ?>" />
<?php endif; ?>
              <h2>System: <input type="text" name="particleSizes" value="<?php echo htmlspecialchars($particleSizes); ?>" /></h2>
              <textarea name="matrix" rows="6"><?php echo htmlspecialchars($matrixInput); ?></textarea>
            </div>

            <div class="output-card">
              <h2>Output</h2>
              <textarea class="output-area" readonly rows="20"><?php echo htmlspecialchars($outputResult); ?></textarea>
            </div>

            <div class="options-card">
              <h2>Parameters</h2>

              <label>Target distance:</label>
              <input type="text" name="targetDistance" placeholder="0.5E-13" />

              <label>Min probability:</label>
              <input type="text" name="minProbForState" placeholder="0" />

              <label>Target states:</label>
              <input type="text" name="targetNumberOfStates" placeholder="N²" />

              <label>Precision:</label>
              <select name="precision">
                <option value="3" <?php echo $precision == "3" ? "selected" : ""; ?>>3</option>
                <option value="6" <?php echo $precision == "6" ? "selected" : ""; ?>>6</option>
                <option value="9" <?php echo $precision == "9" ? "selected" : ""; ?>>9</option>
                <option value="12" <?php echo $precision == "12" ? "selected" : ""; ?>>12</option>
                <option value="15" <?php echo $precision == "15" ? "selected" : ""; ?>>15</option>
              </select>

              <label>Accuracy boost:</label>
              <div class="radio-group">
                <label><input type="radio" name="accuracyBoost" value="1" <?php echo $accuracyBoost == "1" ? "checked" : ""; ?> /> On</label>
                <label><input type="radio" name="accuracyBoost" value="0" <?php echo $accuracyBoost == "0" ? "checked" : ""; ?> /> Off</label>
              </div>

              <div class="action-buttons">
                <button type="submit" name="separate" value="1">Separate</button>
<?php if ($isExample) : ?>
                <span class="tooltip-wrapper" title="Go to Home to input your own matrix">
                  <button type="button" class="secondary-btn" disabled>Randomize</button>
                </span>
<?php else : ?>
                <button type="submit" name="randomize" value="1" class="secondary-btn">Randomize</button>
<?php endif; ?>
              </div>
            </div>
          </div>
        </form>
      </div>
    </main>

<?php include '_footer.php'; ?>
