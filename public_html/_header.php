<!DOCTYPE html>
<html lang="en">
  <head>
    <meta charset="utf-8" />
    <meta name="viewport" content="width=device-width, initial-scale=1" />
    <meta name="color-scheme" content="light dark" />
    <title><?php echo $pageTitle ?? 'State Separator'; ?></title>
    <?php if (isset($pageDescription)) : ?>
    <meta name="description" content="<?php echo htmlspecialchars($pageDescription); ?>" />
    <?php endif; ?>
    <link rel="stylesheet" href="https://cdn.jsdelivr.net/npm/@picocss/pico@2/css/pico.min.css" />
    <link rel="stylesheet" href="style.css?v=20260103b" />
  </head>
  <body>
    <header>
      <div class="container">
        <h1>State Separator <small>v3.0</small></h1>
        <button class="theme-toggle" onclick="toggleTheme()" title="Toggle theme">
          <span class="icon-sun">☀️</span><span class="icon-moon">🌙</span>
        </button>
      </div>
    </header>
    <script>
      function toggleTheme() {
        const html = document.documentElement;
        const current = html.getAttribute('data-theme');
        const next = current === 'dark' ? 'light' : 'dark';
        html.setAttribute('data-theme', next);
        localStorage.setItem('theme', next);
      }
      (function() {
        const saved = localStorage.getItem('theme');
        if (saved) document.documentElement.setAttribute('data-theme', saved);
        else if (window.matchMedia('(prefers-color-scheme: dark)').matches)
          document.documentElement.setAttribute('data-theme', 'dark');
      })();

      // Persist textarea sizes across page loads
      document.addEventListener('DOMContentLoaded', function() {
        const matrixTextarea = document.querySelector('textarea[name="matrix"]');
        if (matrixTextarea) {
          const savedHeight = localStorage.getItem('matrixHeight');
          if (savedHeight) matrixTextarea.style.height = savedHeight;
          new ResizeObserver(() => {
            localStorage.setItem('matrixHeight', matrixTextarea.style.height || matrixTextarea.offsetHeight + 'px');
          }).observe(matrixTextarea);
        }
      });
    </script>
