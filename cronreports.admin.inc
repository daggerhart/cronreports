<?php

/*
 * Table of reports
 */
function theme_cronreports_admin_reports($cronreports_reports){

  $header = array(
    array('data' => 'Status'),
    array('data' => 'Name'),
    array('data' => 'Watchdog types'),
    array('data' => 'Time offset'),
    array('data' => 'Email to'),
    array('data' => 'Edit'),
    array('data' => 'Send'),
  );
  $rows = array();
  foreach($cronreports_reports as $name => $report){
    $rows[] = array(
      'data' => array(
        array('data' => ($report['enabled']) ? 'on' : 'off'),
        array('data' => $report['name']),
        array('data' => $report['watchdog_types']),
        array('data' => $report['time_offset']),
        array('data' => $report['email_to']),
        array('data' => '<a href="/admin/build/cron/reports/edit/'.$report['name'].'?'.drupal_get_destination().'">edit</a>'),
        array('data' => '<a href="/admin/build/cron/reports/send/'.$report['name'].'?'.drupal_get_destination().'">send</a>'),
      )
    );
  }
  
  return theme('table', $header, $rows);
}

/*
 * Admin settings page for all things
 */
function cronreports_admin_form($form_state) {
  drupal_set_title('Reports');
  $form = array();
  $cronreports_reports = variable_get('cronreports_reports', array());

  // debug mesages
  if (variable_get('cronreports_debug_mode', 0)){
    drupal_set_message("Cronreports is in debug mode."); 
  }
  
  // form
  // main
  $form['cronreports_main'] = array(
    '#type' => 'fieldset',
    '#title' => 'Main',
  );
  $form['cronreports_main']['cronreports_debug_mode'] = array(
    '#type' => 'checkbox',
    '#title' => 'Debug mode',
    '#default_value' => variable_get('cronreports_debug_mode', 0),
    '#description' => 'If checked, emails will not be sent.  Instead, a watchdog message for cronreports will be created with details from the report created.',
  );
  $form['cronreports_main']['cronreports_default_email_to'] = array(
    '#type' => 'textfield',
    '#title' => 'Default email to send reports to',
    '#default_value' => variable_get('cronreports_default_email_to', ''),
    '#required' => TRUE,
    '#description' => 'If left blank, cron email reports will not be sent.',
  );
  
  $form['buttons'] = array();
  $form['buttons']['submit'][] = array(
    '#type' => 'submit',
    '#name' => 'save_settings',
    '#value' => 'Save Settings',
  );
  
  // reports
  $form['cronreports_reports'] = array(
    '#type' => 'fieldset',
    '#title' => 'All Reports',
    '#description' => '<a href="/admin/build/cron/reports/add?'.drupal_get_destination().'">Create a new report</a>',
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
  );
  
  if (count($cronreports_reports)) {
    $cronreports_admin_reports = theme('cronreports_admin_reports', $cronreports_reports);
  }
  
  $form['cronreports_reports']['list'] = array(
    '#type' => 'markup',
     '#value' => ($cronreports_admin_reports) ? $cronreports_admin_reports : "No cron reports exist yet.",
  );
  
  return $form;
}

/*
 * Cronreport settings Form Validation
 */
function cronreports_admin_form_validate($form, &$form_state){}

/*
 * Cronreport settings submit handler 
 */
function cronreports_admin_form_submit($form, &$form_state){
  if (isset($form_state['values']['cronreports_default_email_to'])){
    variable_set('cronreports_default_email_to', $form_state['values']['cronreports_default_email_to']);
  }
  if (isset($form_state['values']['cronreports_debug_mode'])){
    variable_set('cronreports_debug_mode', $form_state['values']['cronreports_debug_mode']);
  }
  
  drupal_set_message("Settings have been updated.");
}


/*
 * Add new report form
 */
function cronreports_add_report_form($form_state) {
  drupal_set_title('Add report');
  $form = array();
  
  // existing watchdog types
  $result = db_query("SELECT DISTINCT(`type`) FROM {watchdog}");
  $existing_watchdog_types = array();
  while($row = db_fetch_object($result)){
    $existing_watchdog_types[] = $row->type;
  }
  
  //
  // help
  // 
  $form['cronreports_help'] = array(
    '#type' => 'fieldset',
    '#title' => 'Help',
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
    '#description' => '<p>This report will be listed as "<code>cronreports_{report name}</code>" in Elysia cron.</p>',
  );
  $form['cronreports_help']['current_watchdog_types'] = array(
    '#type' => 'fieldset',
    '#title' => 'Existing watchdog types',
    '#description' => "This is a list of watchdog types that currently exist in drupal's watchdog table.",
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );
  $form['cronreports_help']['current_watchdog_types']['desc'] = array(
    '#type' => 'markup',
    '#value' => '<div style="font-family: monospace; padding-top: 12px;">'.implode(", ", $existing_watchdog_types).'</div>',
  );
  
  //
  // add & edit form
  // 
  $form['cronreports'] = array(
  );
  $form['cronreports']['cronreports_name'] = array(
    '#type' => 'textfield',
    '#title' => 'Report name',
    '#required' => TRUE,
    '#description' => 'Machine safe unique name for this job. Lowercase letters and underscores only.',
    '#default_value' => '',
  );
  
  $form['cronreports']['cronreports_watchdog_types'] = array(
    '#type' => 'textfield',
    '#title' => ' Watchdog types',
    '#required' => TRUE,
    '#description' => 'Comma seperated list of watchdog types to log.',
    '#default_value' => '',
  );
  $form['cronreports']['cronreports_time_offset'] = array(
    '#type' => 'textfield',
    '#title' => 'Time offset',
    '#required' => TRUE,
    '#description' => 'The amount of time this report should sent logs for.  In human readable format. Example: "30 minutes", "1 hour", "1 day", "1 week", etc.',
    '#default_value' => '1 hour',
  );
  $form['cronreports']['cronreports_email_to'] = array(
    '#type' => 'textfield',
    '#title' => 'Email address(es) to send this report to',
    '#description' => 'Comma seperated list of emaill addresses to send this specific report. If left blank the default Cronreports email will be used.',
    '#default_value' => '',
  );
  $form['cronreports']['cronreports_enabled'] = array(
    '#type' => 'checkbox',
    '#title' => t('Enable'),
    '#description' => 'Each report can be enabled &amp; disabled individually after creation.',
    '#default_value' => 0,
  );
  $form['cronreports']['cronreports_skip_empty'] = array(
    '#type' => 'checkbox',
    '#title' => t('Skip when empty'),
    '#description' => 'If checked, this cronreport will not send an email when no watchdog rows are found.',
    '#default_value' => 0,
  );
    
  //
  // buttons
  // 
  $form['buttons']['submit'] = array(
    '#type' => 'submit',
    '#value' => 'Save',
    '#name' => 'save',
  );
  
  return $form;
}

/*
 * Add report validation
 */
function cronreports_add_report_form_validate($form, $form_state){
  $cronreports_reports = variable_get('cronreports_reports', array());
  
  $submitted_values = $form_state['values'];
  
  // name must be machine safe
  $machine_safe = trim(strtolower($submitted_values['cronreports_name']));
  $machine_safe = preg_replace('@[^a-z0-9_]+@','_',$machine_safe);
  if ($submitted_values['cronreports_name'] != $machine_safe){
    form_set_error('cronreports_name', "The Report name provided is not machine safe. Please only use lowercase alpha numeric values and underscores."); 
  }
  // make sure name is existing cron hook or cronreports job name
  if (isset($cronreports_reports[$machine_safe])){
    form_set_error('cronreports_name', "The Report name already exists.  Please choose another name."); 
  }
}

/*
 * Add report submit handler
 */
function cronreports_add_report_form_submit($form, $form_state){
  $report = array(
    'enabled' => $form_state['values']['cronreports_enabled'],
    'name' => $form_state['values']['cronreports_name'],
    'watchdog_types' => $form_state['values']['cronreports_watchdog_types'],
    'time_offset' => $form_state['values']['cronreports_time_offset'],
    'email_to' => $form_state['values']['cronreports_email_to'],
    'skip_empty' => $form_state['values']['cronreports_skip_empty'],
  );
  
  // save it
  $cronreports_reports = variable_get('cronreports_reports', array());
  $cronreports_reports[$report['name']] = $report;
  variable_set('cronreports_reports', $cronreports_reports);
  
  drupal_set_message('Report successfully added');
}

/*
 * Add new report form
 */
function cronreports_edit_report_form($form_state) {
  $arg = arg();
  
  // get the report
  if (isset($arg[5])) {
    $cronreports_reports = variable_get('cronreports_reports', array());
    if (isset($cronreports_reports[$arg[5]])){
      $report = $cronreports_reports[$arg[5]];
    }
  }

  if (isset($report)) {
    // exactly like add form, but with values!
    $form = cronreports_add_report_form();
    drupal_set_title('Edit report: '.$report['name']);
  
    // disable editing report name
    $form['cronreports']['cronreports_name']['#access'] = FALSE;
    // show name
    $form['cronreports']['cronreports_show_name'] = array(
      '#type' => 'markup',
      '#value' => '<div><label>Report name:</label> <code>'.$report['name'].'</code></div>',
      '#weight' => -10,
    );
    
    // set default values
    foreach($report as $key => $value){
      if (isset($form['cronreports']['cronreports_'.$key])){
        $form['cronreports']['cronreports_'.$key]['#default_value'] = $value;
      }
    }
    
    $form['buttons']['delete'] = array(
      '#type' => 'submit',
      '#value' => 'Delete',
      '#name' => 'delete',
    );
    return $form;
  }

  // if the report doesn't exist, 404
  drupal_not_found();
}

/*
 * Add report validation
 */
function cronreports_edit_report_form_validate($form, $form_state){
  $cronreports_reports = variable_get('cronreports_reports', array());
  
  // make sure report exists
  if (!isset($cronreports_reports[$form_state['values']['cronreports_name']])){
    form_set_error('cronreports_show_name', 'Report does not exist.');
  }
}

/*
 * Add report submit handler
 */
function cronreports_edit_report_form_submit($form, $form_state){
  $cronreports_reports = variable_get('cronreports_reports', array());
  
  // see if deleted
  if ($form_state['clicked_button']['#value'] == "Delete"){
    unset($cronreports_reports[$form_state['values']['cronreports_name']]);
  }
  
  // save it
  else {
    $report = array(
      'enabled' => $form_state['values']['cronreports_enabled'],
      'name' => $form_state['values']['cronreports_name'],
      'watchdog_types' => $form_state['values']['cronreports_watchdog_types'],
      'time_offset' => $form_state['values']['cronreports_time_offset'],
      'email_to' => $form_state['values']['cronreports_email_to'],
      'skip_empty' => $form_state['values']['cronreports_skip_empty'],
    );
  
    $cronreports_reports[$report['name']] = $report;
  }
  
  variable_set('cronreports_reports', $cronreports_reports);
  drupal_set_message('Report successfully '.$form_state['clicked_button']['#name'].'d.');
}
