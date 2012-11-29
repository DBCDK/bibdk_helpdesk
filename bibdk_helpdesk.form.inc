<?php

/**
 * Implements hook_form().
 */
function bibdk_helpdesk_form() {

  // Fields related to the question
  $form['question'] = array(
    '#type' => 'fieldset',
    '#tree' => FALSE,
    '#attributes' => array(),
  );
  $form['question']['Title'] = array(
    '#type' => 'hidden',
    '#value' => 'Nyt spørgsmål modtaget fra bibliotek.dk'
  );
  $form['question']['ReceptionMethodId'] = array(
    '#type' => 'hidden',
    '#value' => '6'
  );
  $form['question']['RedirectSuccess'] = array(
    '#type' => 'hidden',
    '#value' => 'http://biblioteksvagten.dk/kvittering.asp'
  );
  $form['question']['TimeLimitId'] = array(
    '#type' => 'select',
    '#title' => t('Time limit'),
    '#description' => t('Select the time limit of your question'),
    '#options' => array(
      t('No time limit'),
      "2 " . t('Hours'),
      "4 " . t('Hours'),
      "24 " . t('Hours'),
      "1 " . t('Day'),
      "1 " . t('Week')
    ),
    '#default_value' => '',
  );
  $form['question']['QuestionText'] = array(
    '#type' => 'textarea',
    '#title' => t('My question is'),
    '#default_value' => '',
    '#required' => TRUE
  );
  $form['question']['PurposeId'] = array(
    "#type" => "fieldset",
    "#after_build" => array("_remove_validation"),
    "#attributes" => array(
      'class' => array('fieldset-notitle'),
    ),
  );
  $form['question']['PurposeId'][] = array(
    '#type' => 'radios',
    '#title' => t('The information will be used for'),
    '#options' => array(
      '1' => t('General interest / hobby'),
      '6' => t('Business'),
    ),
    '#parents' => array('PurposeId'),
    '#spawned' => TRUE,
  );
  $form['question']['PurposeId'][] = array(
    '#type' => 'radios',
    '#title' => t('or in connection with assignment for'),
    '#options' => array(
      '3' => t('Primary school'),
      '2' => t('Gymnasium, EUC, VUC, SOSU or other short-cycle higher education'),
      '4' => t('Medium-cycle higher or higher education'),
      '5' => t('University education or research'),
    ),
    '#spawned' => TRUE,
    '#parents' => array('PurposeId'),
  );

  // Fields related to the customer

  $form['customer'] = array(
    '#type' => 'fieldset',
    '#tree' => FALSE,
  );
  $form['customer']['CustomerName'] = array(
    '#type' => 'textfield',
    '#title' => t('Name'),
    '#default_value' => '',
    '#required' => TRUE
  );
  $form['customer']['CustomerPostalCode'] = array(
    '#type' => 'textfield',
    '#title' => t('PostalCode'),
    '#default_value' => '',
    '#required' => TRUE
  );
  $form['customer']['CustomerEmail'] = array(
    '#type' => 'textfield',
    '#title' => t('Email'),
    '#default_value' => '',
    '#required' => TRUE
  );
  $form['customer']['CustomerPhone'] = array(
    '#type' => 'textfield',
    '#title' => t('Phone'),
    '#default_value' => '',
    '#required' => TRUE
  );

  // Submit
  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Send'),
  );

  return $form;
}


/**
 * Remove required validation from Purpose ID
 */
function _remove_validation($element) {
  _bibdk_helpdesk_disable_validation($element);
  return $element;
}


/*
 * Recursive function.
 * Removes required validation flag for specified form element and all its children.
 */
function _bibdk_helpdesk_disable_validation(&$element) {
  unset($element['#needs_validation']);
  foreach (element_children($element) as $key) {
    _bibdk_helpdesk_disable_validation($element[$key]);
  }
}


/**
 * Validate helpdesk form submission.
 */
function bibdk_helpdesk_form_validate($form, &$form_state) {
  if (!valid_email_address($form_state['values']['CustomerEmail'])) {
    form_set_error("CustomerEmail", t('Email needs to be a valid email', array(), array('context' => 'bibdk_helpdesk:error')));
  }
  if (!isset($form_state['values']['PurposeId'])) {
    form_set_error("PurposeId", t('Please state a purpose', array(), array('context' => 'bibdk_helpdesk:error')));
  }
}


/**
 * Process helpdesk form submission.
 */
function bibdk_helpdesk_form_submit($form, &$form_state) {
  $curl = new MicroCURL();

  $header = array("Content-Type: text/xml; charset=utf-8", "Accept-Charset: utf-8");

  try {
    $url = variable_get('helpdesk_submit_url', '');
    $curl->set_url($url);
    $curl->set_option(CURLOPT_HTTPHEADER, $header);
    $curl->set_post($form_state['values']);
    $curl->get();
  }
  catch (Exception $e) {
    drupal_set_message(t('An error happened : @error', array('@error' => $e), array('context' => 'bibdk_helpdesk')), 'error');
    return;
  }

  if ($curl->has_error()) {
    drupal_set_message(t('Sorry we could not submit your query, please try again later', array(), array('context' => 'bibdk_helpdesk')), 'error');
    watchdog('submit', 'ERROR: cannot submit to helpdesk', $curl->has_error(), WATCHDOG_ERROR);
    return;
  }

  $status = $curl->get_status();
  if ($status['http_code'] == 302) {
    drupal_goto("http://biblioteksvagten.dk/kvittering.asp");
  }
  else {
    drupal_set_message(t('Sorry we could not submit your query, please try again later', array(), array('context' => 'bibdk_helpdesk')), 'error');
    watchdog('submit', 'ERROR: cannot submit to helpdesk', $status, WATCHDOG_ERROR);
  }
}
