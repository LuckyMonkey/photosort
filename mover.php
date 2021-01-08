<?php

//mover php file, this file is given the image, and the name of the folder it needs to be moved to, it will then transport the file there.
//mover uses the rename function in php to move the file

//<a href="page.php?name=var1&folder=var2">Link</a>
//mover.php/?name='image_test.jpg',folder='testFolder'=
$imgMoverSrc= htmlspecialchars($_GET["source"]);
$imgMoverNam= htmlspecialchars($_GET["name"]);
$imgMoverTgt= htmlspecialchars($_GET["folder"]);
rename($imgMoverSrc . $imgMoverNam, $imgMoverTgt . '/' . $imgMoverNam);

setcookie("lastMoveSrc", $imgMoverSrc);
setcookie("lastMoveNam", $imgMoverNam);
setcookie("lastMoveTgt", $imgMoverTgt);
header('Location: index.php');
echo "<!doctype html>\n<html>\n<head>\n<title>Image Sorter</title>\n<link rel='stylesheet' type='text/css' href='./style.css'>\n<link rel='stylesheet' type='text/css' href='./profont.css'>\n</head>\n<body>\n<br /><h1>ImageSorter by Charlie</h1>\n<p id='theMove'>The file '$imgMoverSrc' was moved to the folder '$imgMoverTgt'</p> \n</body> \n</html> \n";





//im fucking stressed out right now god damnit


// have it write in a flat file what functions are performed each step
// setup so php can read that and take the data and undo the last file re-name
// you may want to write an undo function as well!
?>