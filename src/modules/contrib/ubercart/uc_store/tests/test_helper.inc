<?php
/**
 * @file
 * Test functionality used by all Ubercart tests.
 */

/**
 * Defines a base helper class for Ubercart tests.
 */
class UbercartTestHelper extends BackdropWebTestCase {

  /**
   * User with privileges to do everything. */
  protected $adminUser;

  /**
   * Authenticated but unprivileged user. */
  protected $customer;

  /**
   * Test product. */
  protected $product;

  /**
   * Overrides BackdropWebTestCase::setUp().
   *
   * Configures basic Ubercart store components.
   *
   * @param $modules
   *   Optional list of extra modules to install.
   * @param $permissions
   *   Optional list of extra permissions for $this->adminUser.
   */
  protected function setUp($modules = array(), $permissions = array()) {
    // Enable the core Ubercart modules and dependencies, along with any other modules passed as arguments.
    $modules = array_merge(array('entity_ui', 'entity_plus', 'uc_store', 'rules', 'uc_order', 'uc_product', 'uc_cart'), $modules);
    parent::setUp($modules, $permissions);

    // Create a store administrator user account.
    $this->adminUser = $this->backdropCreateUser(array_merge($permissions, array(
      'administer store',
      'administer order workflow',
      'administer product classes',
      'administer product features',
      'administer products',
      'create product content',
      'delete any product content',
      'edit any product content',
      'create orders',
      'view all orders',
      'edit orders',
      'delete orders',
      'unconditionally delete orders',
    )));

    // Create a simple customer user account.
    $this->customer = $this->backdropCreateUser(array('view own orders'));

    // Create a test product.
    $this->product = $this->createProduct(array('uid' => $this->adminUser->uid));

    // In order for Simpletest to grab emails to check their contents, we have
    // to set the mail system to the TestingMailSystem instead of
    // UbercartMailSystem.
    config_set('system.mail', 'uc_cart', 'TestingMailSystem');
    config_set('system.mail', 'uc_order', 'TestingMailSystem');
    config_set('system.mail', 'uc_file', 'TestingMailSystem');
    config_set('system.mail', 'uc_roles', 'TestingMailSystem');
    config_set('system.mail', 'uc_stock', 'TestingMailSystem');
    config_set('system.mail', 'uc_store', 'TestingMailSystem');
  }

  /**
   * Creates a new product.
   */
  protected function createProduct($product = array()) {
    // Set the default required fields.
    $weight_units = array('lb', 'kg', 'oz', 'g');
    $length_units = array('in', 'ft', 'cm', 'mm');
    $product += array(
      'type' => 'product',
      'model' => $this->randomName(8),
      'list_price' => mt_rand(1, 9999),
      'cost' => mt_rand(1, 9999),
      'sell_price' => mt_rand(1, 9999),
      'weight' => mt_rand(1, 9999),
      'weight_units' => array_rand(array_flip($weight_units)),
      'length' => mt_rand(1, 9999),
      'width' => mt_rand(1, 9999),
      'height' => mt_rand(1, 9999),
      'length_units' => array_rand(array_flip($length_units)),
      'pkg_qty' => mt_rand(1, 99),
      'default_qty' => 1,
      'ordering' => mt_rand(-25, 25),
      'shippable' => TRUE,
    );

    return $this->backdropCreateNode($product);
  }

  /**
   * Creates a new product class.
   *
   * Fix this after adding a proper API call for saving a product class.
   */
  protected function createProductClass($data = array()) {
    $product_class = $data + array(
      'pcid' => $this->randomName(8),
      'name' => $this->randomName(8),
      'description' => $this->randomName(8),
    );
    $product_class = (object) $product_class;

    backdrop_write_record('uc_product_classes', $product_class);

    return $product_class;
  }

  /**
   * Helper function to fill-in required fields on the checkout page.
   *
   * @param $edit
   *   The form-values array to which to add required fields.
   */
  protected function populateCheckoutForm($edit = array()) {
    foreach (array('billing', 'delivery') as $pane) {
      $prefix = 'panes[' . $pane . '][' . $pane;
      $key = $prefix . '_country]';
      if (empty($edit[$key])) {
        $country = config_get('uc_store.settings', 'uc_store_country');
        if (!$country) {
          $country = 840;
        }
      }
      else {
        $country = $edit[$key];
      }
      $zone_id = db_query_range('SELECT zone_id FROM {uc_zones} WHERE zone_country_id = :country ORDER BY rand()', 0, 1, array('country' => $country))->fetchField();
      $edit += array(
        $prefix . '_first_name]' => $this->randomName(10),
        $prefix . '_last_name]' => $this->randomName(10),
        $prefix . '_street1]' => $this->randomName(10),
        $prefix . '_city]' => $this->randomName(10),
        $prefix . '_zone]' => $zone_id,
        $prefix . '_postal_code]' => mt_rand(10000, 99999),
      );
    }

    // If the email address has not been set, and the user has not logged in,
    // add a primary email address.
    if (!isset($edit['panes[customer][primary_email]']) && !$this->loggedInUser) {
      $edit['panes[customer][primary_email]'] = $this->randomName(8) . '@example.com';
    }

    return $edit;
  }

  /**
   * Executes the checkout process.
   */
  protected function checkout($edit = array()) {
    $this->backdropPost('cart', array(), 'Checkout');
    $this->assertText(
      t('Enter your billing address and information here.'),
      t('Viewed cart page: Billing pane has been displayed.')
    );

    $edit = $this->populateCheckoutForm($edit);

    // Submit the checkout page.
    $this->backdropPost('cart/checkout', $edit, t('Review order'));
    $this->assertRaw(t('Your order is almost complete.'));

    // Complete the review page.
    $this->backdropPost(NULL, array(), t('Submit order'));

    $order_id = db_query("SELECT order_id FROM {uc_orders} WHERE delivery_first_name = :name", array(':name' => $edit['panes[delivery][delivery_first_name]']))->fetchField();
    if ($order_id) {
      $this->pass(
        t('Order %order_id has been created', array('%order_id' => $order_id))
      );
      $order = uc_order_load($order_id);
    }
    else {
      $this->fail(t('No order was created.'));
      $order = FALSE;
    }

    return $order;
  }

  /**
   * Assert that an email was sent with a specific subject line.
   *
   * @param $pattern
   *   A regular expression to match the subject against.
   *
   * @return
   *   An array containing the most recently sent matching email,
   *   or FALSE if the subject line did not match anything.
   */
  protected function findMail($pattern) {
    foreach (array_reverse($this->backdropGetMails()) as $mail) {
      if (preg_match($pattern, $mail['subject'])) {
        $this->pass(t('E-mail found with subject matching %pattern.', array('%pattern' => $pattern)));
        return $mail;
      }
    }

    $this->fail(t('E-mail not found with subject matching %pattern.', array('%pattern' => $pattern)));
    return FALSE;
  }

  /**
   * Helper function to test for text in a backdrop ajax response.
   *
   * @param $ajax
   *   The ajax response to test.  Must be an array of ajax commands as returned by backdropPostAjax().
   * @param $text
   *   The text to search for.
   * @param $message
   *   The assertion message.
   * @param $not_exists
   *   TRUE to assert that the text is not present.  FALSE (the default) to assert that it is present.
   * @param $plain
   *   TRUE to check only the plain-text contents of the 'data' keys of each 'insert' command (i.e. what would
   *   be inserted into the page).  FALSE to check the complete, json-encoded ajax response.
   */
  protected function assertAjaxHelper($ajax, $text, $message = FALSE, $not_exists = FALSE, $plain = TRUE) {
    $content = '';
    if ($plain) {
      foreach ($ajax as $command) {
        if ($command['command'] == 'insert' && !empty($command['data']) && is_string($command['data'])) {
          $content .= $command['data'];
        }
      }
      $content = filter_xss($content, array());
    }
    else {
      $content = backdrop_json_encode($ajax);
    }
    if (!$message) {
      $message = !$not_exists ? t('"@text" found in ajax response', array('@text' => $text)) : t('"@text" not found in ajax response', array('@text' => $text));
    }
    $this->assert($not_exists == (strpos($content, $text) === FALSE), $message);
  }

  /**
   * Assert that the specified text is present in the plain text version of the html that would
   * be inserted into the page if this ajax response were executed.
   *
   * @param $ajax
   *   The ajax response to test.  Must be an array of ajax commands as returned by backdropPostAjax().
   * @param $text
   *   The text to search for.
   * @param $message
   *   The assertion message.
   */
  protected function assertAjaxText($ajax, $text, $message = FALSE) {
    $this->assertAjaxHelper($ajax, $text, $message, FALSE, TRUE);
  }

  /**
   * Assert that the specified text is not present in the plain text version of the html that would
   * be inserted into the page if this ajax response were executed.
   *
   * @param $ajax
   *   The ajax response to test.  Must be an array of ajax commands as returned by backdropPostAjax().
   * @param $text
   *   The text to search for.
   * @param $message
   *   The assertion message.
   */
  protected function assertNoAjaxText($ajax, $text, $message = FALSE) {
    $this->assertAjaxHelper($ajax, $text, $message, TRUE, TRUE);
  }

  /**
   * Assert that the specified text is present in the raw backdrop ajax response.
   *
   * @param $ajax
   *   The ajax response to test.  Must be an array of ajax commands as returned by backdropPostAjax().
   * @param $text
   *   The text to search for.
   * @param $message
   *   The assertion message.
   */
  protected function assertAjaxRaw($ajax, $text, $message = FALSE) {
    $this->assertAjaxHelper($ajax, $text, $message, FALSE, FALSE);
  }

  /**
   * Assert that the specified text is not present in the raw backdrop ajax response.
   *
   * @param $ajax
   *   The ajax response to test.  Must be an array of ajax commands as returned by backdropPostAjax().
   * @param $text
   *   The text to search for.
   * @param $message
   *   The assertion message.
   */
  protected function assertNoAjaxRaw($ajax, $text, $message = FALSE) {
    $this->assertAjaxHelper($ajax, $text, $message, TRUE, FALSE);
  }

  /**
   * Extends backdropPostAjax() to replace additional content on the page after an ajax submission.
   *
   * BackdropWebTestCase::backdropPostAjax() will only process ajax insertions which don't have a 'selector' attribute,
   * because it's not easy to convert from a jQuery selector to an XPath.  However, ubercart uses many simple,
   * id-based selectors, and these can be converted easily (eg: '#my-identifier' => '//*[@id="my-identifier"]').
   * This helper method post-processes the command array returned by backdropPostAjax() to perform these insertions.
   *
   * @see BackdropWebTestCase::backdropPostAjax()
   */
  protected function ucPostAJAX($path, $edit, $triggering_element, $ajax_path = NULL, array $options = array(), array $headers = array(), $form_html_id = NULL, $ajax_settings = NULL) {
    $commands = parent::backdropPostAJAX($path, $edit, $triggering_element, $ajax_path, $options, $headers, $form_html_id, $ajax_settings);
    $dom = new DOMDocument();
    @$dom->loadHTML($this->backdropGetContent());
    foreach ($commands as $command) {
      if ($command['command'] == 'insert' && isset($command['selector']) && preg_match('/^\#-?[_a-zA-Z]+[_a-zA-Z0-9-]*$/', $command['selector'])) {
        $xpath = new DOMXPath($dom);
        $wrapperNode = $xpath->query('//*[@id="' . substr($command['selector'], 1) . '"]')->item(0);
        if ($wrapperNode) {
          // ajax.js adds an enclosing DIV to work around a Safari bug.
          $newDom = new DOMDocument();
          $newDom->loadHTML('<div>' . $command['data'] . '</div>');
          $newNode = $dom->importNode($newDom->documentElement->firstChild->firstChild, TRUE);
          $method = isset($command['method']) ? $command['method'] : $ajax_settings['method'];
          // The "method" is a jQuery DOM manipulation function. Emulate
          // each one using PHP's DOMNode API.
          switch ($method) {
            case 'replaceWith':
              $wrapperNode->parentNode->replaceChild($newNode, $wrapperNode);
              break;

            case 'append':
              $wrapperNode->appendChild($newNode);
              break;

            case 'prepend':
              // If no firstChild, insertBefore() falls back to
              // appendChild().
              $wrapperNode->insertBefore($newNode, $wrapperNode->firstChild);
              break;

            case 'before':
              $wrapperNode->parentNode->insertBefore($newNode, $wrapperNode);
              break;

            case 'after':
              // If no nextSibling, insertBefore() falls back to
              // appendChild().
              $wrapperNode->parentNode->insertBefore($newNode, $wrapperNode->nextSibling);
              break;

            case 'html':
              foreach ($wrapperNode->childNodes as $childNode) {
                $wrapperNode->removeChild($childNode);
              }
              $wrapperNode->appendChild($newNode);
              break;
          }
        }
      }
    }
    $content = $dom->saveHTML();
    $this->backdropSetContent($content);
    $this->verbose('Page content after ajax submission:<hr />' . $this->content);
    return $commands;
  }

}
