$(document).ready(function(){
       
    $.ajaxSetup({ cache: true });

    // Fill item from restAPI
    get_schema_list();

    $(".asset_detail").hide();
});

var get_schema_list = function () {
    $.get("/cm/lib/adsk/cm/type?List", function (data, textStates) {
        var len = data.items.length;
        var array = ["Ceramic", "Concrete", "Generic", "Glazing", "Hardwood", "MasonryCMU", "Metal",
            "MetallicPaint", "PlasticVinyl", "PrismLayered", "PrismMetal", "PrismOpaque", "PrismTransparent",
            "SolidGlass", "Stone", "WallPaint"];

        for (i = 0; i < len; i++) {
            var element = data.items[i];
            element;
            var index = element.indexOf("Schema");
            var name = "";
            if (index > 0)
                name = element.substr(0, index);

            if ($.inArray(name, array) != -1) {

                var link = "/cm/lib/adsk/cm/asset/" + element + "?Get";
                /*
                var a = $('<a>', {
                    html: name,
                    //href: '#',
                    href: link,
                    data: {}
                    //data: { list: link }
                }).appendTo('#filter');
                */
                var a = $('<a>', {
                    html: name,
                    href: '#',
                    data: { list: link }
                }).appendTo('#filter');
            }
           
        }
        schema_list_ready();
    }, "json");
}


function schema_list_ready() {

    $('#filter a').click(function () {
        $('.asset_detail').hide();
        var link = $(this);
        get_object_list($(this).data('list'));
        link.addClass('active').siblings().removeClass('active');
        //$('#stage').quicksand(link.data('list').find('li'));
    });

    $('#filter a:first').click();
}


var get_object_list_quicksand = function(path){
    $.get(path, function (data) {
        var len = data.items.length;

        var items = $('#stage li');
        var item_count = items.length;
        var ul = $('<ul>', { 'class': 'hidden' });

        for (var i = 0; i < len; ++i) {
            var asset = data.items[i];
            var thumbnail;
            if (asset.thumbnail.length != 0 && asset.thumbnail[0].length > 0) {
                thumbnail  = '<img src="' + asset.thumbnail[0] + '"/>';
            }
            var node = "<li data-tags=\"" + asset.UIName + "\"" + " id=\"" + data.assets[i] + "\"" + " data-id=\"" + item_count+i
                + "\">" + thumbnail + asset.UIName + "</li>"

            ul.append(node);
        }
        ul.appendTo('#container');

        var itemlist = ul.find('li');
        $('#stage').quicksand(itemlist);
        object_list_ready();
    }, "json");
}

var get_object_list = function (path) {
    $.get(path, function (data) {
        var len = data.items.length;

        var items = $('#stage li');
        var item_count = items.length;
        //var ul = $('<ul>', { 'class': 'hidden' });

        $('ul li').remove();
        var ul = $('<ul>');
        for (var i = 0; i < len; ++i) {
            var asset = data.items[i];
            var thumbnail;
            if (asset.thumbnail.length != 0 && asset.thumbnail[0].length > 0) {
                thumbnail = '<img src="' + asset.thumbnail[0] + "?Get" + '"/>';
            }
            var node = "<li data-tags=\"" + asset.UIName + "\"" + " id=\"" + data.assets[i]
                + "\"" + " schema_path=\"" + data.path
                + "\">" + thumbnail + asset.UIName + "</li>"

            ul.append(node);
        }
        $('#container ul').remove();
        ul.appendTo('#container');

        object_list_ready();
    }, "json");
}

var object_list_ready = function () {
    var items = $('#stage li');
    $('ul li').click(function () {
        $(this).addClass("active").siblings().removeClass("active");
        show_detail(this);
    });

    $('ul li').hover(function () {
        $(this).addClass("li_hover").siblings().removeClass("li_hover");
    }, 
    function () {
        $(this).removeClass("li_hover");
    }
    );
}

var show_detail = function (e) {
    var id = e.getAttribute("id");
    var schema_path = e.getAttribute("schema_path");

    var asset_path = "/cm" + schema_path + '/' + id + "?Get";
    $('#back_to_material').attr('data_path', asset_path);
    do_show_detail(asset_path);
    $('#back_to_category').show();
    $('#back_to_material').hide();

    $('#back_to_category').click(back_to_category);
    $('#back_to_material').click(back_to_material);
    $('.sub_button').hover(back_to_link_hover, back_to_link_hover_removed);
}

var do_show_detail = function (asset_path) {
    $("#asset_data").hide();
    $.get(asset_path, function (data) {

        var asset_data = $("#asset_data");
        var text = '<table><tbody>'
			+ '<thead><tr><th>Property</th><th>Value</th></tr></thead>';
        var odd = false;
        for (var i = 0 in data) {
            text += odd ? '<tr class="odd">' : '<tr class="even">'
            text += '<td>' + i + '</td><td>'
				+ obj_to_td(data[i], odd) +
				'</td></tr>';
            odd = !odd;
        }

        text += '</tbody></table>';
        asset_data.html(text);


        $(".link").click(function () {
            do_show_detail('/cm/' + this.innerHTML + "?Get");
            $('#back_to_material').show();  // Always allow to go back to material
        });

        $('#container ul').css({ height: "320px" });
        
        $(".asset_detail").show();
        asset_data.fadeIn(500);
    }, "json");

}

var obj_to_td = function (obj, odd) {
    //var text = odd ? '<div class="value odd">' : '<div class="value even">';
    var text = '<div>';
    if ((obj instanceof Array) || (typeof obj === 'object')) {
        text += '<table><tbody>';
        for (var i in obj) {
            text += "<tr><td>" + obj_to_td(i, odd) + "</dr>";
            text += "<td>" + obj_to_td(obj[i], odd) + "</td></tr>";
        }
        text += '</tbody></table>';
    }
    else {
        if (typeof obj === 'string') {
            if (obj.indexOf("/cm/lib/adsk/cm/resource/") === 0)
                text += '<img src="' + obj + '"/>';
            else if (obj.indexOf("/lib/adsk/cm/asset/") === 0) {
                text += '<span class="link">' + obj + '</span>';
            }
            else
                text += obj;
        }
        else
            text += obj;
    }
    text += '</div>'
    return text;
}

var back_to_category = function () {
    $('.asset_detail').hide();
    $('#container ul').css({ height: "640px" });
    $('#container ul li.active').removeClass('active');
}

var back_to_material = function () {
    var asset_path = $('.breadcrumb #back_to_material').attr("data_path");
    do_show_detail(asset_path);
    $('#back_to_material').hide();
}

var back_to_link_hover = function () {
    $('.sub_button').addClass("sub_button_hover");
}

var back_to_link_hover_removed = function () {
    $('.sub_button').removeClass("sub_button_hover");
}
