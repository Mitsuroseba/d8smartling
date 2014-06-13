<?php
require_once(dirname(__FILE__) . '/smartling_content_parser_interface.inc');

class SmartlingImageUrlParser implements ISmartlingContentParser {
  protected $regexp = '~\s+(href|src|action|longdesc)="([^"]+)~i';
  protected $processors;
  protected $field_name;
  protected $entity;

  public function __construct(array $processors) {
    $this->processors = $processors;
  }

  /*
   * Wrapper method for retrieving global params.
   */
  protected function getGlobalSettings() {
    global $base_path;
    global $base_url;

    return array($base_path, $base_url);
  }

  /*
   * Determines whether we are dealing with an internal or external link.
   *
   * @param array $matches Array of strings that matched the regexp.
   *
   * @return array
   */
  protected function getContext($matches) {
    // Get the base path.
    list($base_path, $base_url) = $this->getGlobalSettings();

    $base_url_parts = @parse_url($base_url . '/');
    $local_paths_exploded[] = array('path' => $base_url_parts['path']);
    $local_paths_exploded[] = array('path' => $base_url_parts['path'], 'host' => $base_url_parts['host']);

    // If it appears the path is a scheme-less URL, prepend a scheme to it.
    // parse_url() cannot properly parse scheme-less URLs. Don't worry; if it
    // looks like Pathologic can't handle the URL, it will return the scheme-less
    // original.
    // @see https://drupal.org/node/1617944
    // @see https://drupal.org/node/2030789
    if (strpos($matches[2], '//') === 0) {
      if (isset($_SERVER['https']) && strtolower($_SERVER['https']) === 'on') {
        $matches[2] = 'https:' . $matches[2];
      }
      else {
        $matches[2] = 'http:' . $matches[2];
      }
    }
    // Now parse the URL after reverting HTML character encoding.
    // @see http://drupal.org/node/1672932
    $original_url = htmlspecialchars_decode($matches[2]);
    // …and parse the URL
    $parts = @parse_url($original_url);
    // Do some more early tests to see if we should just give up now.
    if ($parts === FALSE || (isset($parts['scheme']) && !in_array($parts['scheme'], array('http', 'https', 'files', 'internal')))
      || (isset($parts['fragment']) && count($parts) === 1)) {
      // Give up by "replacing" the original with the same.
      return $matches[0];
    }

    if (isset($parts['path'])) {
      // Undo possible URL encoding in the path.
      // @see http://drupal.org/node/1672932
      $parts['path'] = rawurldecode($parts['path']);
    }
    else {
      $parts['path'] = '';
    }

    // Check to see if we're dealing with a file.
    // @todo Should we still try to do path correction on these files too?
    if (isset($parts['scheme']) && $parts['scheme'] === 'files') {
      // Path Filter "files:" support. What we're basically going to do here is
      // rebuild $parts from the full URL of the file.
      $new_parts = @parse_url(file_create_url(file_default_scheme() . '://' . $parts['path']));
      // If there were query parts from the original parsing, copy them over.
      if (!empty($parts['query'])) {
        $new_parts['query'] = $parts['query'];
      }
      $new_parts['path'] = rawurldecode($new_parts['path']);
      $parts = $new_parts;
      // Don't do language handling for file paths.
      $cached_settings['is_file'] = TRUE;
    }
    else {
      $cached_settings['is_file'] = FALSE;
    }

    // Let's also bail out of this doesn't look like a local path.
    $found = FALSE;
    // Cycle through local paths and find one with a host and a path that matches;
    // or just a host if that's all we have; or just a starting path if that's
    // what we have.
    foreach ($local_paths_exploded as $exploded) {
      // If a path is available in both…
      if (isset($exploded['path']) && isset($parts['path'])
        // And the paths match…
        && strpos($parts['path'], $exploded['path']) === 0
        // And either they have the same host, or both have no host…
        && (
          (isset($exploded['host']) && isset($parts['host']) && $exploded['host'] === $parts['host'])
          || (!isset($exploded['host']) && !isset($parts['host']))
        )
      ) {
        // Remove the shared path from the path. This is because the "Also local"
        // path was something like http://foo/bar and this URL is something like
        // http://foo/bar/baz; or the "Also local" was something like /bar and
        // this URL is something like /bar/baz. And we only care about the /baz
        // part.
        $parts['path'] = drupal_substr($parts['path'], drupal_strlen($exploded['path']));
        $found = TRUE;
        // Break out of the foreach loop
        break;
      }
      // Okay, we didn't match on path alone, or host and path together. Can we
      // match on just host? Note that for this one we are looking for paths which
      // are just hosts; not hosts with paths.
      elseif ((isset($parts['host']) && !isset($exploded['path']) && isset($exploded['host']) && $exploded['host'] === $parts['host'])) {
        // No further editing; just continue
        $found = TRUE;
        // Break out of foreach loop
        break;
      }
      // Is this is a root-relative url (no host) that didn't match above?
      // Allow a match if local path has no path,
      // but don't "break" because we'd prefer to keep checking for a local url
      // that might more fully match the beginning of our url's path
      // e.g.: if our url is /foo/bar we'll mark this as a match for
      // http://example.com but want to keep searching and would prefer a match
      // to http://example.com/foo if that's configured as a local path
      elseif (!isset($parts['host']) && (!isset($exploded['path']) || $exploded['path'] === $base_path)) {
        $found = TRUE;
      }
    }

    return array('external' => !$found);
  }

  /*
   * Processes each item that was found by a regexp in a parse method.
   *
   * @return string
   */

  protected function processorExecutor($match) {
    $context = $this->getContext($match);

    foreach($this->processors as $processor) {
      $processor->process($match, $context, $this->lang, $this->field_name, $this->entity);
    }

    return " {$match[1]}=\"{$match[2]}";
  }

  public function parse($content, $lang, $field_name, $entity) {
    $this->field_name = $field_name;
    $this->entity = $entity;
    $this->lang = $lang;

    $content = preg_replace_callback($this->regexp, array($this, 'processorExecutor'), $content);

    return $content;
  }
}