<?php
$pageTitle = 'Complex Separable State - State Separator';
$pageDescription = 'Example: A separable state with complex matrix elements.';
$currentPage = 'examples';
$isExample = true;
$exampleTitle = 'Complex Separable State';
$exampleDesc = 'Separable state with complex matrix elements';
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
              <h2 class="tooltip-wrapper" title="Go to Home to input your own matrix">System: <input type="text" name="particleSizes" value="2   2" readonly /></h2>
              <div class="tooltip-wrapper" title="Go to Home to input your own matrix">
              <textarea name="matrix" rows="8" readonly>0.3        0          0          0
0          0.2        0.1+0.1i   0
0          0.1-0.1i   0.4        0
0          0          0          0.1</textarea>
              </div>
            </div>
<?php include '_options.php'; ?>
          </div>
        </form>
      </div>
    </main>

<?php include '_footer.php'; ?>
