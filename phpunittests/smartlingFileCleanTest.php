<?php
require_once(dirname(__FILE__) . '/../smartling.utils.inc');
/**
 * @file
 * Tests for smartling.
 */

/**
 * SmartlingFileCleanTest.
 */
class SmartlingFileCleanTest extends \PHPUnit_Framework_TestCase {

  /**
   * Test info.
   *
   * @return array
   *   Return test info.
   */
  public static function getInfo() {
    return array(
      'name' => 'Smartling clean file',
      'description' => 'Test Smartling file cleaning to avoid path traversal',
      'group' => 'Smartling UnitTests',
    );
  }

  /**
   * Test clean file.
   */
  public function testCleanFile() {
    $filename = smartling_clean_filename("");
    $this->assertEquals($filename, "", 'Path traversal test for empty filenames.');

    $filename = smartling_clean_filename("./../asd.abc");
    $this->assertEquals($filename, "asd.abc", 'Path traversal test for: "./../asd.abc"');

    $filename = smartling_clean_filename("./../asd.abc", TRUE);
    $this->assertEquals($filename, "_/__/asd.abc", 'Path traversal test for: "./../asd.abc" (path enabled)');

    $filename = smartling_clean_filename("qwe.ert\n\n.pdf");
    $this->assertEquals($filename, "qwe_ert__.pdf", 'Path traversal test for: "qwe.ert\n\n.pdf"');

    $filename = smartling_clean_filename("%u002e%u002e%u2215qwrtyu.htm");
    $this->assertEquals($filename, "_u002e_u002e_u2215qwrtyu.htm", 'Path traversal test for: "%u002e%u002e%u2215qwrtyu.htm"');

    $filename = smartling_clean_filename("liuerg");
    $this->assertEquals($filename, "liuerg", 'Path traversal test for: "liuerg"');

    $filename = smartling_clean_filename("\n\n");
    $this->assertEquals($filename, "", 'Path traversal test for: "\n\n"');

    $filename = smartling_clean_filename("dir1/dir2/dir3/dir4", TRUE);
    $this->assertEquals($filename, "dir1/dir2/dir3/dir4", 'Path traversal test for: "dir1/dir2/dir3/dir4"');
  }

}