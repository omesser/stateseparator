<?php
$pageTitle = 'State Separator - Quantum State Analysis Tool';
$pageDescription = 'A numerical algorithm to check whether a multi-Qudit quantum state is separable or entangled.';
$currentPage = 'home';
include '_header.php';
include '_nav.php';
?>

    <main>
      <div class="container">
        <form method="post" action="./calculate.php">
          <div class="calculator-grid">
            <div class="matrix-card">
              <div class="example-header">
                <h1>Input State Matrix</h1>
                <p>Enter system dimension and matrix input. See <a href="documentation.php">documentation</a> for details.</p>
              </div>
              <h2>System: <input type="text" name="particleSizes" value="2   2" /></h2>
              <p>For 2-Qubits: 2 2 | For Qubit+Qutrit: 2 3</p>
              <textarea name="matrix" rows="5">0.25  0     0     0
0     0.25  0     0
0     0     0.25  0
0     0     0     0.25</textarea>
              <small>Example: Maximally-mixed state</small>
            </div>

<?php include '_options.php'; ?>
          </div>
        </form>
      </div>
    </main>

<?php include '_footer.php'; ?>
