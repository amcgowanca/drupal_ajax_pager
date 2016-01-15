<?php
/**
 * @file
 * ajax_pager.api.inc
 */

/**
 * Registers an Ajax pager type with the system for processing and rendering.
 *
 * The returned value is an array, keyed by the pager type's machine name. Each
 * element should contain the following properties for correct operation:
 *   * callback: The callback used by the Ajax pager's handler for rebuilding
 *       the particular set of data and renderable content. The returned value
 *       of this callback should be an array with two elements: `selector` and
 *       `data`.
 *          * selector: is the ID attribute of the wrapper element which houses
 *             the paginated data element and pager element. This should be
 *             prefixed with a pound symbol (#).
 *          * data: the render array of the elements (paginated data and pager).
 *
 * @return array
 *   Returns information about the different pager types.
 *
 * @see ajax_pager_element_callback().
 */
function MODULE_ajax_pager_info() {
  return array(
    'MODULE_ajax_pager_name' => array(
      'callback' => 'MODULE_ajax_pager_callback',
    ),
  );
}