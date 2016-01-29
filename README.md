## Drupal Ajax Pager

The Ajax Pager module aims to provide a simple programmatic interface and a replacement to Drupal core's pager mechanisms for enabling ajaxified pagers.

### Installation

#### Common installation

* Download the module and place it within the `sites/all/modules` directory under the directory named `ajax_pager`.
* Install the module either via the Administrative interface or by using Drush, `drush en ajax_pager --yes`.

### Examples and usage

Each Ajax Pager type must be registered to so that the Ajax Pager request handler and the `AjaxPagerDefault` select query extender and correctly process and handle pagination. This is done through the implementation of `HOOK_ajax_pager_info()` which is responsible for returning an array of pager information, keyed by the Ajax Pager's machine name with the following properties:

* `callback`: A callable function implemented by the module registering the Ajax Pager which is invoked during and Ajax request to the pager handler for rebuilding the paginated content and pager element, returning it in an array with two properties:
    * `selector`: the the wrapper element's ID attribute
    * `data`: the render array of the rebuilt paginated data

```
/**
 * Implements hook_ajax_pager_info().
 */
function ajax_pager_example_ajax_pager_info() {
  return array(
    'ajax_pager_example' => array(
      'callback' => 'ajax_pager_example_callback',
    ),
  );
}

/**
 * Callback for the Ajax Pager type for rebuild and rendering.
 *
 * @return array
 *   The selector and data to rebuild from the Ajax request.
 *
 * @see ajax_pager_example_ajax_pager_info().
 * @see _ajax_pager_example_build().
 */
function ajax_pager_example_callback($arg) {
  $info = _ajax_pager_example_table_info();
  if (!isset($info[$arg])) {
    return FALSE;
  }

  $return = array(
    'selector' => '#ajax-pager-example-' . $arg,
    'data' => _ajax_pager_example_build($arg, $info[$arg]),
  );
  return $return;
}
```

An Ajax Pager is initialized similarily to Drupal core's pager via a `SelectQuery` object by using the `extend` method. However, due to the complexities required for an ajax pager in Drupal (without patching Drupal core), a few additional things need to be done:

* We need to extend the `SelectQuery` object and specify `AjaxPagerDefault` as the extender.

```
$query = db_select('ajax_pager_example', 't')->extend('AjaxPagerDefault');
```

* After we've initialized the `$query` to be an instance of the `AjaxPagerDefault` extender, we must also specify the ajax pager type by invoking the `setAjaxPagerType($name, $element = NULL)` method.

```
$query->setAjaxPagerType('ajax_pager_example');
```

The `setAjaxPagerType` method implemented by `AjaxPagerDefault` also excepts a second parameter which specifies the element indicator. The element indicator is an integer value equal to or greater than zero that is used for indicating the pager instance on the page should more than one pager instance exist. For example, if there is only a single Ajax Pager on a page no `$element` indicator is needed to be specified. However, should more than one ajax pager exist on a single page, you need to specify the instance indicator similarily how Drupal core's `theme('pager', array('#element' => $element))` would, to ensure proper processing and re-theming. This is because when multiple ajax pagers exist on a single page, the query string parameter which is used to indicate the the page number contains a comma separated value. For example:

* If only one pager exists on the page, the query string may look like this: `?ajax_pager_example=2` which would indicate that the pager should render page 3 (note that this is a zero-based, meaning page one is technically known to the system as a value of `0`).
* If multiple pagers exist on the page, the query string may look like this: `?ajax_pager_example=0,2` would which indicate that the second pager on the page should render page 3 data, whereas the first page would be on page 1.

Rendering the ajax pager is achieved by invoking the theme function and specifying the ajax pager type in the specified `$variables`. For example:

```
$build['pager'] = theme('ajax_pager', array('#ajax_pager_type' => 'ajax_pager_example'));
```

In addition to the `#ajax_pager_type` parameter of the specified `$variables`, you can also specify the following:

* `#ajax_pager_element`: indicates the element indicator.
* `#arguments`: an array of URL arguments which are passed to the Ajax Pager module's ajax request handler and are forwarded to the Ajax Pager type's callback for rebuilding. These arguments should be URL friendly.
* `#parameters`: an associative array of key-value pairs that are appended to each pager link as query-string parameters. Note that these values will not override the parameters implemented by the pager.
* `#quantity`: The number of pages to list within the pager. The default value is `9`.

#### Pagers without a SelectQuey

There are specific cases where pagers are needed that do not require interaction with the database. For example, if you were specifically requesting and displaying data from an REST API, you may want to only show five records at a time from the API if the API allows for paginated results and total count values to be easily retrieved.

Ajax Pager instances can be initialized and rendered with full functionality without the need of leveraging the `AjaxPagerDefault` select query extender. To do so, we can make use of the `ajax_pager_initialize(...)` function that performs the standard setup. This function accepts four parameters:

* `$ajax_pager_type_name`: The machine name of the ajax pager type to initialize.
* `$element`: The integer value indicating the pager element. Default value is NULL which would perform automatic detection.
* `$total_items`: The total number of items in the data collection to be paginated through.
* `$limit`: The total number of items to display per page.

The returned value of this function upon executing it is the current page for this pager instance. Remember that this is a zero-based value, therefore if the current page is page one, the first page, the returned value would be zero (`0`).

```
function ajax_pager_initialize($ajax_pager_type_name, $element = NULL, $total_items, $limit) {
  global $ajax_pager_data;

  if (!isset($ajax_pager_data)) {
    $ajax_pager_data = array();
  }

  $ajax_pager_info = ajax_pager_get_info($ajax_pager_type_name);
  if (!$ajax_pager_info) {
    throw new Exception(t('The Ajax Pager type, @type, does not exist.', array('@type' => $ajax_pager_type_name)));
  }

  $element_count = &ajax_pager_element_count($ajax_pager_info['name']);
  if (NULL !== $element && $element_count < $element) {
    $element_count = $element;
  }

  $ajax_pager_info['element'] = $element_count;
  $element_count++;

  if (!isset($ajax_pager_data[$ajax_pager_info['name']][$ajax_pager_info['element']])) {
    $current_page = ajax_pager_find_page($ajax_pager_info);
    $ajax_pager_info['total_items'] = $total_items;
    $ajax_pager_info['total_pages'] = ceil($total_items / $limit);
    $ajax_pager_info['current_page'] = max(0, min($current_page, ((int) $ajax_pager_info['total_pages']) - 1));
    $ajax_pager_info['limit'] = $limit;
    $ajax_pager_data[$ajax_pager_info['name']][$ajax_pager_info['element']] = $ajax_pager_info;
  }
  return $ajax_pager_data[$ajax_pager_info['name']][$ajax_pager_info['element']]['current_page'];
}
```

It is up to you to limit the data that is displayed to the user by selecting only specific values for that page. This is easily achieved knowing the current page value and the number of items being displayed per page. Lastly, the pager element can be easily rendered onto the page using the standard mechanism as described above.

### License

This Drupal module is licensed under the [GNU General Public License](./LICENSE.md) version 2.
