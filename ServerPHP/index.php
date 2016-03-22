<?php
//--------------------
$key="EB0C68BF96E8C26635D3450293D2FC501A63A09924FE90A7BD916AC521FDE0AA";
$hmac_zeros="0000000000000000000000000000000000000000000000000000000000000000";
$sha_base="160000";
//--------------------
header("HTTP/1.1 200 OK");
header("Content-Type: application/json");
header("Cache-Control: no-cache, no-store, must-revalidate");
header("Connection: close");

date_default_timezone_set("CET");

$Data['responsejson']['emulator']['version'] = "0.2";
$Data['responsejson']['emulator']['contact'] = "http://github.com/latchdevel/crypto-iot";

$Data['responsejson']['server']['scheme'] = $_SERVER['REQUEST_SCHEME'];
$Data['responsejson']['server']['host'] = $_SERVER['SERVER_NAME'];
$Data['responsejson']['server']['method'] = $_SERVER['REQUEST_METHOD'];
$Data['responsejson']['server']['uri'] = $_SERVER['REQUEST_URI'];

$req_auth = getallheaders()["Authorization"];

if (!empty($req_auth)){
  $req_auth_type = explode(" ", $req_auth)[0];
  if (!empty($req_auth_type)){
    $Data['responsejson']['auth']['type'] = $req_auth_type;
    $req_auth_keys = str_replace(" ","",str_replace("$req_auth_type","",$req_auth));
    preg_match_all('/\s*([^=,\s]+)\s*=\s*(?:"((?:[^"]|"")*)"|([^,"]*))\s*,?/',$req_auth_keys,$authorization_values);
    $req_auth_parsed = array_combine ( $authorization_values[1] , $authorization_values[2] );
//--------------------
    if((!empty($req_auth_parsed["id"]))){
      $id = base64_decode($req_auth_parsed["id"],true);
      if ($id){
        $Data['responsejson']['auth']['id'] = strtoupper(trim(chunk_split(bin2hex($id),2,"")));
      }else{
        $Data['responsejson']['error']['authorization']['id'] = "base64 invalid";
      }
    }else{
      $Data['responsejson']['error']['authorization']['id'] = "missing";
    }
//--------------------
    if((!empty($req_auth_parsed["nonce"]))){
      $nonce = base64_decode($req_auth_parsed["nonce"],true);
      if ($nonce){
        $Data['responsejson']['auth']['nonce'] = strtoupper(trim(chunk_split(bin2hex($nonce),2,"")));
      }else{
        $Data['responsejson']['error']['authorization']['nonce'] = "base64 invalid";
      }
    }else{
      $Data['responsejson']['error']['authorization']['nonce'] = "missing";
    }
//--------------------
    if((!empty($req_auth_parsed["base"]))){
      $base = base64_decode($req_auth_parsed["base"],true);
      if ($base){
        $Data['responsejson']['auth']['base'] = strtoupper(trim(chunk_split(bin2hex($base),2,"")));
      }else{
        $Data['responsejson']['error']['authorization']['base'] = "base64 invalid";
      }
    }else{
      $Data['responsejson']['error']['authorization']['base'] = "missing";
    }
//--------------------
    if((!empty($req_auth_parsed["signature"]))){
      $signature = base64_decode($req_auth_parsed["signature"],true);
      if ($signature){
        $Data['responsejson']['auth']['signature'] = strtoupper(trim(chunk_split(bin2hex($signature),2,"")));
        }else{
          $Data['responsejson']['error']['authorization']['signature'] = "base64 invalid";
        }
      }else{
          $Data['responsejson']['error']['authorization']['signature'] = "missing";
      }
//--------------------
    if((!empty($_GET["timestamp"])) && (isset($_GET["timestamp"])) ){
      $req_timestamp = $_GET["timestamp"];
      $req_timestamp_txt = date(DATE_RFC1123,$req_timestamp);
      if (!empty($req_timestamp_txt)){
        $my_timestamp = time();
        $my_timestamp_txt = date(DATE_RFC1123,$my_timestamp);
        $Data['responsejson']['timestamp']['request']=$req_timestamp_txt;
        $Data['responsejson']['timestamp']['current']=$my_timestamp_txt;
        $drift = round(abs($my_timestamp- $req_timestamp) / 60,0);
        $Data['responsejson']['timestamp']['drift']=$drift;
        if ($drift>=5){
          $Data['responsejson']['error']['timestamp']['drift'] = "too much";
        }
      }else{
        $Data['responsejson']['error']['timestamp'] = "invalid";
      }
    }else{
      $Data['responsejson']['error']['timestamp'] = "missing";
    }
      }else{
        $Data['responsejson']['error']['authorization']['type'] = "missing";
      }
    }else{
      $Data['responsejson']['error']['authorization'] = "emply";
    }
//--------------------
  if(!isset($Data['responsejson']['error'])){

    // Generate request string to compute SHA1 (20 bytes)
    $tosha1="{$Data['responsejson']['server']['method']} {$Data['responsejson']['server']['uri']} HTTP/1.1\r\n";

    // Compute SHA-1 of request string. Out 20 bytes in HEX format
    $sha1= strtoupper(hash('sha1', $tosha1));

    // Generate 55 bytes in HEX format to compute SHA-256
    $tosha256 = bin2hex($nonce) . $sha1 . $sha_base;

    // Compute SHA256 from previus 55 bytes. Out 32 bytes in HEX format
    $sha256 = strtoupper(hash('sha256', hex2bin($tosha256)));

    // Generate 88 bytes in HEX format to compute HMAC-256
    $tohmac= $hmac_zeros . $sha256 . bin2hex($base);

    // Compute HMAC-256 from previus 88 bytes. Out 32 bytes in BIN format.
    $hmac = hash_hmac ( 'sha256', hex2bin($tohmac) , hex2bin($key), true  );

    $Data['responsejson']['crypt']['request'] = $tosha1;
    $Data['responsejson']['crypt']['req-sha1'] = $sha1;
    $Data['responsejson']['crypt']['hmac-256'] = strtoupper(bin2hex($hmac));

    if ($hmac!=$signature){
      $Data['responsejson']['error']['crypt']['signature'] = "invalid";
      $Data['responsejson']['crypt']['authenticated'] = false;
    }else{
      $Data['responsejson']['crypt']['authenticated'] = true;
      //
      // Surprise!!!!
      //
    }
}
//--------------------
$json_string = json_encode($Data, JSON_PRETTY_PRINT|JSON_UNESCAPED_SLASHES);

$length=strlen($json_string);

$length=$length+1;

header("Content-length: $length");

print("{$json_string}\n");

exit(0);
?>
