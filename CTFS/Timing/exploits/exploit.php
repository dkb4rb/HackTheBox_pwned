<?php
$upload_dir = "images/uploads/";
$file = "exploit.jpg";

$file_name = md5('$file_hash'. time()) . '_' . $file;
$target_file = $upload_dir . $file_name;
echo $file_name;
echo PHP_EOL;
?>
