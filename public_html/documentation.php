<?php
$pageTitle = 'Documentation - State Separator';
$pageDescription = 'Documentation for the State Separator quantum state analysis tool.';
$currentPage = 'docs';
include '_header.php';
include '_nav.php';
?>

    <main>
      <div class="container documentation">
        <div class="page-title">
          <h1>Documentation</h1>
          <p>Complete guide to using the State Separator tool</p>
        </div>

        <article>
          <h1>1. Overview</h1>
          <div class="highlight-box">
            <p><strong>The State Separator implements a probabilistic numerical algorithm that checks whether a given multi-Qudit state is separable or entangled.</strong></p>
          </div>

          <p>The algorithm is based on <em>"Geometrical aspects of entanglement"</em>, Physical Review A 74, 012313 (2006), by Jon Magne Leinaas, Jan Myrheim and Eirik Ovrum.</p>

          <p>The state separator also performs a <strong>Peres-Horodecki partial-transpose test</strong> on the matrix. This test is always a necessary condition for separability, and is also sufficient for 2×2 and 2×3 sized systems.</p>

          <p>The tool will find the nearest separable state and its decomposition to pure product states, providing the distance from the target matrix.</p>

          <h2>Quick Start</h2>
          <p>Simply copy the density matrix to be tested to the main program window, enter the Qudit dimensions (e.g., "2 2" for 2-Qubits) and click <strong>"Separate"</strong>.</p>

          <h2>Result Interpretation</h2>
          <ul>
            <li><strong>Entangled (Peres Test)</strong> — The input state failed the Peres-Horodecki test</li>
            <li><strong>Separable</strong> — Target distance reached (default: 0.5E-13)</li>
            <li><strong>Might be entangled</strong> — Distance is less than 0.0005</li>
            <li><strong>Most likely entangled</strong> — Distance exceeds 0.0005</li>
          </ul>
        </article>

        <article>
          <h1>2. Usage Guide</h1>

          <h2>2.1 System Dimensions</h2>
          <p><strong>Format:</strong> Space-separated integers representing each particle's dimension.</p>
          <ul>
            <li><code>2 2</code> — Two qubits (2×2 system)</li>
            <li><code>2 3</code> — One qubit + one qutrit (2×3 system)</li>
            <li><code>2 2 3</code> — Two qubits + one qutrit</li>
          </ul>
          <p><em>Note: The product of dimensions must equal the matrix order.</em></p>

          <h2>2.2 Matrix Input</h2>
          <p>Enter your density matrix in the text area. Elements should be separated by whitespace, with each row on a new line.</p>

          <p><strong>Complex number formats supported:</strong></p>
          <pre>+2.3-i0.2    -2.3-0.2i    2.3-i0.2
i2.3-0.2     -i2.3        2.3i
3            -2.3         i+0.1</pre>

          <div class="highlight-box">
            <p><strong>Requirements:</strong> The matrix must be Hermitian, positive semi-definite, and have unit trace. Warnings will be displayed if these conditions aren't met.</p>
          </div>

          <h2>2.3 Optional Parameters</h2>

          <h2>Target Distance</h2>
          <p>The threshold for considering the input and approximation as equal. Default: <code>0.5E-13</code></p>

          <h2>Minimum Probability</h2>
          <p>States with probability below this threshold are discarded. Default: <code>0</code></p>

          <h2>Target Number of States</h2>
          <p>Maximum product states in the decomposition. Default: <code>N²</code></p>

          <h2>Output Precision</h2>
          <p>Number of decimal digits displayed (3, 6, 9, 12, or 15). This only affects display, not calculation accuracy.</p>

          <h2>Accuracy Boost</h2>
          <p>Increases algorithm iterations for higher precision at the cost of computation time.</p>
          <ul>
            <li><strong>Off (default):</strong> M = N² + N, R = 100</li>
            <li><strong>On:</strong> M = N³, R = 1000</li>
          </ul>
        </article>

        <article>
          <h1>3. Algorithm</h1>
          <p>The algorithm performs an iterative search for a close approximation to the input matrix within the separable matrices subspace.</p>

          <p>Starting from the maximally mixed state, the system iteratively adds pure states to the mixed-state approximation. Each pure state maximizes the projection of the distance vector between the current best-approximation and the input matrix onto the separable subspace.</p>

          <p>Key optimization techniques:</p>
          <ul>
            <li><strong>Quadratic programming</strong> for distance minimization</li>
            <li><strong>LDLT Cholesky decomposition</strong> (via Eigen library) for efficient solving</li>
            <li><strong>Iterative refinement</strong> of tensor-product pure states</li>
          </ul>

          <p>Reference: <em>"Geometrical aspects of entanglement"</em>, Physical Review A 74, 012313 (2006)</p>
        </article>

        <article>
          <h1>4. Accuracy</h1>

          <h2>Input Accuracy</h2>
          <p>Floating-point representation is limited to 14 significant digits. Entering more is redundant.</p>

          <h2>Algorithm Accuracy</h2>
          <p>Using Werner states W = q·I + (1-q)·B as a benchmark, the system achieves accuracy of 0.5E-4 in parameter q (can detect separability at q=0.3332 vs. theoretical threshold of 1/3).</p>
        </article>

        <article>
          <h1>5. Time Complexity</h1>
          <p>Worst case: <strong>O(R × n × M × N³)</strong></p>
          <ul>
            <li><strong>R</strong> — TraceOut refinement steps</li>
            <li><strong>n</strong> — Number of particles</li>
            <li><strong>M</strong> — Maximum main iterations</li>
            <li><strong>N</strong> — Matrix order</li>
          </ul>
        </article>

        <article>
          <h1>6. Version History</h1>
          <p>The State Separator began in 2012 as an undergraduate project at the Technion Physics Department by Naftaly Shalev, completed in 2014 by Oded Messer under J. Avron's guidance.</p>
          <p>It provides students, researchers, and quantum information enthusiasts an accessible tool for testing quantum state separability.</p>
        </article>

        <article>
          <h1>7. Licensing</h1>
          <p>The system uses C++ with the <a href="http://eigen.tuxfamily.org/index.php?title=Main_Page#License" target="_blank" rel="noopener">Eigen library</a> (LGPL3+/MPL2).</p>
          <p>All rights reserved to the Department of Physics, Technion. You may use and distribute this program freely.</p>
          <p><em>Disclaimer: Neither the Technion nor the authors are responsible for any outcomes from using this system.</em></p>
        </article>

        <article>
          <h1>8. Matrix Format Reference</h1>
          <p>Supported complex number formats:</p>
          <pre>[+/-][real][+/-][i][imag]     Example: +2.3-i0.2
[+/-][real][+/-][imag][i]     Example: -2.3-0.2i
[+/-][i][imag][+/-][real]     Example: +i2.3-0.2
[+/-][real]                   Example: -2.3
[+/-][imag][i]                Example: +2.3i
[+/-][i][imag]                Example: -i2.3</pre>
          <p><em>Note: Standalone <code>i</code> is interpreted as <code>1i</code> (e.g., 3+i = 3+1i)</em></p>
        </article>
      </div>
    </main>

<?php include '_footer.php'; ?>
