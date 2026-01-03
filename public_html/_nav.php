    <nav>
      <div class="container">
        <ul>
          <li><a href="index.php"<?php echo ($currentPage ?? '') === 'home' ? ' class="active"' : ''; ?>>Home</a></li>
          <li>
            <a href="#"<?php echo ($currentPage ?? '') === 'examples' ? ' class="active"' : ''; ?>>Examples ▾</a>
            <ul>
              <li><a href="simpleSeparable.php">Simple Separable</a></li>
              <li><a href="complexSeparable.php">Complex Separable</a></li>
              <li><a href="barelySeparable.php">Barely Separable</a></li>
              <li><a href="slightlyEntangled.php">Slightly Entangled</a></li>
              <li><a href="bellState.php">Bell State</a></li>
              <li><a href="wState.php">W State</a></li>
              <li><a href="boundEntangled.php">Bound Entangled</a></li>
            </ul>
          </li>
          <li><a href="documentation.php"<?php echo ($currentPage ?? '') === 'docs' ? ' class="active"' : ''; ?>>Docs</a></li>
          <li><a href="contact.php"<?php echo ($currentPage ?? '') === 'contact' ? ' class="active"' : ''; ?>>Contact</a></li>
        </ul>
      </div>
    </nav>
