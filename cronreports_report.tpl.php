<?php

  // loop through and build the email body
  $body = '';
  foreach($report['results'] as $log){
    $body.= date('d M Y G:i:s', $log->timestamp)." - ".t($log->message, unserialize($log->variables)). "\n\n";
  }
  print ($body) ? $body : "no logs found for the types given: ".$report['watchdog_types'].' : between '.date('M d Y g:i:s', $report['start_time']).' and '.date('M d Y g:i:s');  
