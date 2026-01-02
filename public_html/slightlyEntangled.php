<?php
$pageTitle = 'Slightly Entangled State - State Separator';
$pageDescription = 'Example: A Werner state just above the entanglement threshold.';
$currentPage = 'examples';
$isExample = true;
$exampleTitle = 'Slightly Entangled State';
$exampleDesc = 'Werner state q=0.34 — just above threshold (q>1/3)';
include '_header.php';
include '_nav.php';
?>

    <main>
      <div class="container">
        <form method="post" action="./calculate.php">
          <div class="calculator-grid">
            <div class="matrix-card">
              <div class="example-header">
                <h1><?php echo $exampleTitle; ?></h1>
                <p><?php echo htmlspecialchars($exampleDesc); ?></p>
              </div>
              <input type="hidden" name="exampleTitle" value="<?php echo htmlspecialchars($exampleTitle); ?>" />
              <input type="hidden" name="exampleDesc" value="<?php echo htmlspecialchars($exampleDesc); ?>" />
              <h2 class="tooltip-wrapper" title="Go to Home to input your own matrix">System: <input type="text" name="particleSizes" value="2   2" readonly /></h2>
              <div class="tooltip-wrapper" title="Go to Home to input your own matrix">
              <textarea name="matrix" rows="8" readonly>0.335 0     0     0.17
0     0.165 0     0
0     0     0.165 0
0.17  0     0     0.335</textarea>
              </div>
            </div>
<?php include '_options.php'; ?>
          </div>
        </form>
      </div>
    </main>

<?php include '_footer.php'; ?>
