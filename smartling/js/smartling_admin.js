/**
 * @file
 * Custom javascript.
 */

(function($) {

  Drupal.behaviors.smartlingProgressbar = {
    attach: function(context, settings) {

      var progress = '';
      var is_progressbar = $('.view-smartlig-report .views-field-progress').attr('class');
      if (typeof is_progressbar !== 'undefined') {
        $('.view-smartlig-report tbody .views-field-progress').each(function() {
          progress = $(this).html();
          $(this).empty();
          $(this).progressbar({
            value: parseInt(progress)
          });
        });
      }
    }
  }

})(jQuery);
