<?php
if (isset($_GET["tra"])){
  file_put_contents("traffic.txt", $_GET["tra"]);

  date_default_timezone_set('Europe/Istanbul');
  file_put_contents("traffic_log.txt", date(DATE_RFC822)." - ".$_GET["tra"]."\n", FILE_APPEND);
}
?>