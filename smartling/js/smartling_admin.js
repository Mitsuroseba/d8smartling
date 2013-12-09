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
          var progress_string = '<div class="progress-val">' + progress + '</div>';
          $(this).empty();
          $(this).append(progress_string);
          $(this).css({'position': 'relative'});
          $(this).find('.progress-val').css({'display': 'inline-block', 'width': '100%', 'height': '100%', 'text-align': 'center', 'position': 'absolute', 'left': '0'});
          $(this).progressbar({
            value: parseInt(progress)
          });
        });
      }
    }
  }

})(jQuery);
