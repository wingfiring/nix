$(document).ready(function(){
	$.ajaxSetup({cache: true});
	show_tree();
	show_workspace();
	//$('#save_modification').hide();
});


var show_tree = function(){
    $('#schema_list').jstree( {
        "core" : {
            'data' : {
                'url' : function (node) {
                    //return node.id === '#' ? "/cs2/rp/submissions?/lib/adsk/cm/1.0" : "/cs2/rp/cat?" + node.id;
                    return node.id === '#' ? "/cs2/rp/submissions?/lib/adsk/cm/1.0" : "/cs2/rp/cat?" + node.a_attr.href;
                },
                'data' : function (node) {
                    return "";
                },
                'dataFilter' : function(json, type){
                    var data = JSON.parse(json)
		    var items = data.items;
		    var len = items.length;
                    var i;
                    var ret = [];
                    var arg = data.arg;
                    for (i = 0; i < len; ++i){

			var version_id = items[i].version;
			var id = version_id != null ? version_id : items[i].id;
                        var text = version_id != null ? version_id : items[i].description;
			
			var href = id;
			if (version_id != null)
			    href = href + "&" + '/lib/adsk/cm/1.0';
			else
			{
			    if (arg == null)
				href = href + "&" + '/lib/adsk/cm/1.0';
			    else
				href = href + "&" + arg + '/' + text;
			}

                        var folder = {
                            "id": id,
                            "dir": text,
                            "text": text,
                            "children": true,
			    "a_attr":{"href": href} 
                        };
			ret.push(folder);
                    }
                    
                    return ret;
                }
            }
        }
    })
    .on('changed.jstree', function (e, data) {
        if(data && data.selected && data.selected.length) {
		show_json('/cs2/rp/content?' + data.node.a_attr.href);
        }
        else {
            $('#data .content').hide();
            $('#data .default').html('Select a file from the tree.').show();
        }
})};


var show_workspace = function(){
    $('#workspace_view').jstree( {
        "core" : {
            'data' : {
                'url' : function (node) {
                    //return node.id === '#' ? "/cs2/rp/submissions?/lib/adsk/cm/1.0" : "/cs2/rp/cat?" + node.a_attr.href;
                    return "/cs2/rp/workspace?/mygit/stage";
                },
                'data' : function (node) {
                    return "";
                },
                'dataFilter' : function(json, type){
                    var data = JSON.parse(json)
		    var items = data.Added;
		    var len = items.length;
                    var i;
                    var ret = [];
                    var arg = data.arg;
                    for (i = 0; i < len; ++i){

			/*var version_id = items[i].version;
			var id = version_id != null ? version_id : items[i].id;
                        var text = version_id != null ? version_id : items[i].description;
			
			var href = id;
			if (version_id != null)
			    href = href + "&" + '/lib/adsk/cm/1.0';
			else
			{
			    if (arg == null)
				href = href + "&" + '/lib/adsk/cm/1.0';
			    else
				href = href + "&" + arg + '/' + text;
			}*/


			var text = items[i];
			var href = text;
                        var folder = {
                            "id": text,
                            "file": text,
                            "text": text,
                            "children": true,
			    "a_attr":{"href": href} 
                        };
			ret.push(folder);
                    }
                    
                    return ret;
                }
            }
        }
    })
    .on('changed.jstree', function (e, data) {
        /*if(data && data.selected && data.selected.length) {
		show_json('/cs2/rp/content?' + data.node.a_attr.href);
        }
        else {
            $('#data .content').hide();
            $('#data .default').html('Select a file from the tree.').show();
        }*/
	var text_area = $('#json_text_area');
	var value = text_area[0].value;

//var content = "lee hahah    tata";
//text_area[0].value = content;
	//var html = text_area.innerHTML;

})};

var hide_pannel = function(){
	//$(".asset_detail").hide();
	//$(".asset_table").hide();
	//$('#json_view').hide();
}

var commit = function(){
	var input = $("#commit_input");
	var value = input[0].value;
	var url = '/cs2/rp/commit?' + value;
	$.post(url, function(data){
	});
}

var save_workspace = function(){
	var text_area = $('#json_text_area');
	var value = text_area[0].value;
	//var path = '/lib/adsk/cm/1.0/prism/materialappearance/Glass/Smooth/Prism-150';
	var json_path = document.getElementById('json_path');
	var path = json_path.href;
	var pos = path.indexOf('#');
	if (pos != -1)
	{
	    path = path.substr(pos+1, path.length-pos-1);
	}

	var url = '/cs2/rt/object?' + path;
	$.post(url, value, function(data){
		},"json");
}

var show_json = function(path){
		$.get(path, function(data){
			//hide_pannel();
			var json_view = $('#json_view');
			json_view.html('<pre ><code class="language-js">' + JSON.stringify(data, null, '  ') + '</code></pre>');

			json_view.show();
			// Dump json data into text_area
			var text_area = $('#json_text_area');
			var content = JSON.stringify(data.result, null, '  ');
			text_area[0].value = content;

			var json_path = document.getElementById('json_path');
			json_path.href = '#' + data.arg;

	}, "json");
}

// Global data stored for later use.
