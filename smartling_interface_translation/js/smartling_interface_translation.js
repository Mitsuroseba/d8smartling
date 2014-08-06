/**
 * @file
 * Custom javascript.
 */

(function ($) {

    Drupal.behaviors.smartlingPoProgressbar = {
        attach: function (context, settings) {

            var progress = '';
            var is_progressbar = $('.smartling-po-progress-table');
            if (typeof is_progressbar !== 'undefined') {
                $('.smartling-po-progress-table .smartling-po-progress-item').each(function () {
                    progress = $(this).html();
                    var progress_string = '<div class="progress-val">' + progress + '</div>';
                    $(this).empty();
                    $(this).append(progress_string);
                    $(this).css({'display': 'block', 'position': 'relative'});
                    $(this).find('.progress-val').css({'display': 'inline-block', 'width': '100%', 'text-align': 'center', 'position': 'absolute', 'left': '0'});
                    $(this).progressbar({
                        value: parseInt(progress)
                    });
                });
            }
        }
    }

})(jQuery);
