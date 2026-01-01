<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">

<?php
$separate = $_POST['separate'] ?? null;
$randomize = $_POST['randomize'] ?? null;

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
if ($separate) {
    $matrix = $_POST['matrix'] ?? "";
}
?>

<html>
<head>
<link rel="stylesheet" href="style.css" type="text/css">

<!-- Google Analytics -->
<!--script>
  (function(i,s,o,g,r,a,m){i['GoogleAnalyticsObject']=r;i[r]=i[r]||function(){
  (i[r].q=i[r].q||[]).push(arguments)},i[r].l=1*new Date();a=s.createElement(o),
  m=s.getElementsByTagName(o)[0];a.async=1;a.src=g;m.parentNode.insertBefore(a,m)
  })(window,document,'script','//www.google-analytics.com/analytics.js','ga');

  ga('create', 'REPLACEME', 'auto');
  ga('require', 'displayfeatures');
  ga('send', 'pageview');
</script-->
<!-- End Google Analytics -->

</head>
<body>
    <div id="header">
        <h1>Welcome to the State Separator (V2.0)</h1>
    </div>
    <div id="navigation">
        <ul id="navbar">
            <li><a href="index.html">Home</a></li>
            <li><a href="#head">Examples</a>
                <ul>
                    <li><a href="simpleSeparable.html">Simple Separable State</a><li>
                    <li><a href="complexSeparable.html">Complex Separable State</a><li>
                    <li><a href="barelySeparable.html">Barely Separable State</a><li>
                    <li><a href="slightlyEntangled.html">Slightly Entangled State</a><li>
                    <li><a href="bellState.html">Bell State</a><li>
                    <li><a href="wState.html">W State</a><li>
                    <li><a href="BoundEntanglementState.html">Bound-entangled State</a><li>
                </ul>
            </li>
            <li><a href="documentation.html">Documentation</a></li>
            <li><a href="contact.html">Contact</a></li>
        </ul>
    </div>
    <div id="main">
        <form method="post" action="./calculate.php">
            <div id="data">
                <h1>The system:</h1>
                <input type="text" name="particleSizes" value="<?php echo $particleSizes?>" style="width:100%" Rows=1 />
                <h1>The density matrix to be analyzed:</h1>
                <textarea name="matrix" rows="30" wrap="off"><?php
                if ($separate) {
                    echo "$matrix\n\n";

                    exec("./NSeparator \"$particleSizes\" \"$matrix\" \"$targetDistance\" \"$minProbForState\" \"$targetNumberOfStates\" \"$precision\" \"$accuracyBoost\"", $separatorOutput, $returnValue);
                    foreach ($separatorOutput as $val) {
                        echo "$val\n";
                    }
                } elseif ($randomize) {
                    exec("./NRandomizer \"$particleSizes\" \"$precision\"", $randomizerOutput, $returnValue);
                    foreach ($randomizerOutput as $val) {
                        echo "$val\n";
                    }
                }
                ?></textarea>
            </div>
            <div id="options">
                <h1>Optional parameters:</h1>
                <p>
                    Target distance:<br><input type="text" name="targetDistance" value="" maxlength=20><br><br>
                    Minimum probability per state:<br><input type="text" name="minProbForState" value="" maxlength=20><br><br>
                    Target number of states:<br><input type="text" name="targetNumberOfStates" value="" maxlength=20><br><br>
                    Output precision: <select name="precision">
                        <option value="3">3</option>
                        <option value="6">6</option>
                        <option value="9">9</option>
                        <option value="12">12</option>
                        <option value="15">15</option>
                    </select><br><Br>
                    Accuracy boost: <br>
                    <input type="radio" style="width:10%" name="accuracyBoost" value="1"> On <br>
                    <input type="radio" style="width:10%" name="accuracyBoost" checked="checked"  value="0">  Off (default) <br>
                    <br>
                    <input type="submit" name="separate" value=" Separate "/><br>
                    <input type="submit" name="randomize" value=" Randomize "/>
                </p>
            </div>
        </form>
    </div>
    <div id="footer">
        <address>Developed at the Technion - Israel Institute of Technology, Haifa, Israel</address>
    </div>
</body>
</html>
