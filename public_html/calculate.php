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

if ($separate) {
    $matrix = $_POST['matrix'] ?? "";
    $outputMarkers = array('The system is', 'ERROR:', 'The Eigenvalues');
    $cleanMatrix = $matrix;
    foreach ($outputMarkers as $marker) {
        $pos = strpos($cleanMatrix, $marker);
        if ($pos !== false) {
            $cleanMatrix = substr($cleanMatrix, 0, $pos);
        }
    }
    $matrixInput = trim($cleanMatrix);
    exec("./NSeparator \"$particleSizes\" \"$matrixInput\" \"$targetDistance\" \"$minProbForState\" \"$targetNumberOfStates\" \"$precision\" \"$accuracyBoost\"", $separatorOutput, $returnValue);
    $outputResult = implode("\n", $separatorOutput);
} elseif ($randomize) {
    exec("./NRandomizer \"$particleSizes\" \"$precision\"", $randomizerOutput, $returnValue);
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
