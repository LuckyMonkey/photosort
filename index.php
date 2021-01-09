<?php

// HTML Template, styles, font, header Included.
$appTitle = 'PHP Typing Image Sorter';
echo "<!doctype html>\n<html>\n<head>\n<title>$appTitle</title>\n<link rel='stylesheet' type='text/css' href='./style.css'>\n</head>\n<body>\n<h1 class='bgGrey'>$appTitle</h1>\n";

// This folder contains all of the images that need to be sorted
// example = "images/";
$imagesDirectory = "images/";
$sortedDirectory = "sorted/";

// Tell us what the last moved file was 
// TODO:Hide the out put if cookie is not set
// echo "<p id='theMove'>The file ".$_COOKIE['lastMoveNam']." was moved to the folder ".$_COOKIE['lastMoveTgt']."</p>";

// Make an array out of the images inside of the imagesDirectory
$imagesArray = glob($imagesDirectory.'*.{jpg,jpeg,png}', GLOB_BRACE);

$filecount = 0;
if ($imagesArray){$filecount = count($imagesArray); }
// HTML Filecount Output
echo "<p id='fileCount'>There are <span>$filecount</span> images left to sort!</p>";

// Get the first image in the imagesArray array
$focusImage = substr($imagesArray[0], strlen($imagesDirectory));
$focusImagePath = ($imagesArray[0]);

// Output the highlight image not as a thumbnail first
echo "<img alt='$imagesDirectory$focusImage' src='$imagesDirectory$focusImage' />\n<p id='exifData'>";
$exif = exif_read_data($focusImagePath, 0, true);
foreach ($exif as $key => $section) {
    foreach ($section as $name => $val) {
        echo "$key.$name: $val<br />\n";
    }
}

echo "</p>";
echo '<br />';
// For every item in the imagesArray use imageFile as the variable
foreach($imagesArray as $imageFile) {
// Get thumbnail data from each image so the app doesn't crash with thousands of images
$imageThumb = exif_thumbnail($imageFile, $width, $height, $type);   
    if ($imageThumb!==false) {
        // Output the thumbnail as base64 to not create more unneeded files
        echo "<img alt='$imageFile' width='$width' height='$height' src='data:image/jpg;base64,".base64_encode($imageThumb)."'> \n";
    } else {
        // If thumbnail doesn't exist within EXIF data output full sized image // TODO error handling for thumbnails that do not work
        echo "<img alt='$imageFile (no thumbnail available for this image)' src='".$imageFile."'> \n";
    }
};

// Create an array of keyboard shortcuts
$kbKeys = array("q","w","e","r","t","y","u","i","o","p","a","s","d","f","g","h","j","k","l");

// Define Z as the key for the undo function 
$undoKey = "z";

// create array from folders
$dirs = array_filter(glob($sortedDirectory.'*'), 'is_dir');

// Compare the number of values in each array so they can be combined
function combine_arr($a, $b)
{
    $acount = count($a);
    $bcount = count($b);
    $size = ($acount > $bcount) ? $bcount : $acount;
    $a = array_slice($a, 0, $size);
    $b = array_slice($b, 0, $size);
    return array_combine($a, $b);
}

// Combine the two arrays so each folder is matched with a keyboard key
$combined = combine_arr($kbKeys, $dirs);
echo "<div id='kbShorts'>";
function printer($v, $k) {
    global $focusImage, $sortedDirectory, $imagesDirectory;
    $shortFolder = substr($v, strlen($sortedDirectory));
    // Create a link to be sent to mover.php with the Source, Name, and Target of the image.
    echo "<a id='go2$k' href='mover.php?source=$imagesDirectory&name=$focusImage&folder=$v'><span>$k</span><br />$shortFolder</a>"; 
};

// Display each keyboard shorcut
array_walk($combined, "printer");
// Display undo key following folder names
echo "<a href='mover.php?undo'><span>Z</span><br />undo</a>"; 

echo "<script deffer>
const text = '";
foreach ($kbKeys as $v) {echo $v;};
echo "z';
const keys = text.split(''), smoke = {};
keys.forEach(key => smoke[key] = () => document.getElementById(`go2\${key}`).click());
document.addEventListener('keypress', ({key}) => {if (smoke[key]) smoke[key]()});
</script>
";

// End the document
echo "</div>\n</body>\n</html>";
?>