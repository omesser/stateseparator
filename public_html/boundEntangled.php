<?php
$pageTitle = 'Bound-Entangled State - State Separator';
$pageDescription = 'Example: A bound-entangled state that passes the Peres test but is still entangled.';
$currentPage = 'examples';
$isExample = true;
$exampleTitle = 'Bound-Entangled State';
$exampleDesc = 'Passes Peres test but is entangled (3×3 system)';
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
                <p><?php echo $exampleDesc; ?></p>
              </div>
              <input type="hidden" name="exampleTitle" value="<?php echo htmlspecialchars($exampleTitle); ?>" />
              <input type="hidden" name="exampleDesc" value="<?php echo htmlspecialchars($exampleDesc); ?>" />
              <h2 class="tooltip-wrapper" title="Go to Home to input your own matrix">System: <input type="text" name="particleSizes" value="3   3" readonly /></h2>
              <div class="tooltip-wrapper" title="Go to Home to input your own matrix">
              <textarea name="matrix" rows="14" readonly> 0.097222  -0.097222  -0.027778   0.027778  -0.027778   0.027778  -0.027778   0.027778  -0.027778
-0.097222   0.097222   0.027778  -0.027778   0.027778  -0.027778   0.027778  -0.027778   0.027778
-0.027778   0.027778   0.097222   0.027778  -0.027778  -0.097222  -0.027778   0.027778  -0.027778
 0.027778  -0.027778   0.027778   0.097222   0.027778  -0.027778  -0.097222  -0.027778   0.027778
-0.027778   0.027778  -0.027778   0.027778   0.222222   0.027778  -0.027778   0.027778  -0.027778
 0.027778  -0.027778  -0.097222  -0.027778   0.027778   0.097222   0.027778  -0.027778   0.027778
-0.027778   0.027778  -0.027778  -0.097222  -0.027778   0.027778   0.097222   0.027778  -0.027778
 0.027778  -0.027778   0.027778  -0.027778   0.027778  -0.027778   0.027778   0.097222  -0.097222
-0.027778   0.027778  -0.027778   0.027778  -0.027778   0.027778  -0.027778  -0.097222   0.097222</textarea>
              </div>
            </div>
<?php include '_options.php'; ?>
          </div>
        </form>
      </div>
    </main>

<?php include '_footer.php'; ?>
