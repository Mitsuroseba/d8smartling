
(function ($) {
    Drupal.behaviors.smartlingCheckAll = {
        attach: function (context, settings) {
            //alert(settings.smartling.checkAllId);
            for (var i=0; i < settings.smartling.checkAllId.length; ++i) {
                //alert(settings.smartling.checkAllId[i]);
                $(settings.smartling.checkAllId[i]).prepend( "<a href='#' id='smartling-check-all-" + i + "'>Check/uncheck all</p>" );

                $('#smartling-check-all-' + i).click(function() {
                    if ($(this).attr('checked_all') == "0") {
                        $(this).parent().find(':checkbox').attr("checked", true).each(function() { this.click() });
                        $(this).attr('checked_all', 1);
                    }
                    else {
                        $(this).parent().find(':checkbox').attr("checked", false).each(function() { this.click() });
                        //$(settings.smartling.checkAllId[i]).find(':checkbox').attr("checked", false).each(function() { this.click() });
                        $(this).attr('checked_all', 0);
                    }
                    return false;
                });
            }

        }
    };
})(jQuery);