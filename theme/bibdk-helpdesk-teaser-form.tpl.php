<?php

/**
 * @file
 * Theme implementation for helpdesk teaser.
 */
?>

<?php
  print drupal_render($form['bibdk_helpdesk']);
?>

<div id="helpdesk-teaser-image" class="<?php print $image_css; ?>"></div>

<?php
  print drupal_render_children($form);
?>

