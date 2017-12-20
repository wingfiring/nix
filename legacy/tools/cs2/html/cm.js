var dirLocated = (window.location.hash.length == 0);
$(document).ready(function(){
	$.ajaxSetup({cache: true});
	hide_pannel();

	init_tree();

	// init hashchange for history change
	$(window).bind('hashchange',function(e){
		console.log("hash changed: ", document.location.href);

		var jst = $('#schema_list').jstree();
		var danchor = window.location.hash.replace(/^#d=/, "");
		if (danchor.length != 0 && danchor.indexOf('#') == -1){
			if ($('#assetTable').attr("namespace-path") == danchor){
				if (!$('#assetTable').is(":visible")){
					hide_pannel();
					$('#assetTable_parent').show()
				}
				return;
			}

			jst.deselect_all();
			jst.select_node(danchor, false);

			//hide_pannel();
			//$('#assetTable_parent').show()
			return;
		}
		var oanchor = window.location.hash.replace(/^#o=/, "");
		if (oanchor.length != 0 && oanchor.indexOf('#') == -1){
			if ($('#assetDetailTable_parent').attr("obj-or-type-path") == oanchor){
				if (!$('#assetDetailTable_parent').is(":visible")){
					hide_pannel();
					$('#assetDetailTable_parent').show()
				}
				return;
			}
			var obj_name = oanchor.replace(/^.+\//, "");
			danchor = oanchor.substr(0, oanchor.length - obj_name.length - 1);
			jst.deselect_all();
			jst.select_node(danchor, true);
			show_obj_detail(oanchor);
		}
	});

	//var assetTable = $('#assetTable').dataTable();
	//var keyTable = new $.fn.dataTable.KeyTable( assetTable );
	//apply_returnkey_event(keyTable);
});


var init_tree = function(){
	$('#schema_list').jstree( {
		"core" : {
			'data' : {
				'url' : function (node) {
					return node.id === '#' ? "/cs2/rt/namespace?/lib/adsk/cm/1.0" : "/cs2/rt/namespace?" + node.id;
				},
				'data' : function (node) {
					return "";
                },
                'dataFilter' : function(json, type){
                    var data = JSON.parse(json)
                    var ns = data.result.namespace;
                    var len = ns.length;
                    var i;
                    var ret = [];
                    
                    for (i = 0; i < len; ++i){
						var ns_id = data.arg + "/" + ns[i];
                        var folder = {
                            "id": ns_id,
                            "text": ns[i],
                            "children": true,
							"a_attr":{"href": "#d=" + ns_id} 
                        };
						ret.push(folder);
                    }
                    
                    return ret;
                }
            }
        }
    })
	.on('model.jstree', function(e, data) {
		if (!dirLocated){
			var nodes = data.nodes;
			var len = nodes.length;

			var danchor = window.location.hash.replace(/^#d=/, "");
			var oanchor = window.location.hash.replace(/^#o=/, "");

			if (danchor.length != 0 && danchor.indexOf('#') == -1){
				for (var i = 0; i < len; i++){
					var node = nodes[i];
					if (danchor == node){
						data.instance.select_node(node, false, false, e);
						dirLocated = true;
						break;
					}
					else if (danchor.startsWith(node + "/")){
						data.instance.open_node(node);
					}
				}
			}
			if (oanchor.length != 0 && oanchor.indexOf('#') == -1){
				var obj_name = oanchor.replace(/^.+\//, "");
				danchor = oanchor.substr(0, oanchor.length - obj_name.length - 1);

				for (var i = 0; i < len; i++){
					var node = nodes[i];
					if (danchor == node){
						show_obj_detail(oanchor);
						dirLocated = true;
						break;
					}
					else if (oanchor.startsWith(node + "/")){
						data.instance.open_node(node);
					}
				}
			}
		}
	})
	.on('changed.jstree', function (e, data) {
		if (data.action && data.action == "select_node"){
			$('#assetTable').attr("namespace-path", data.node.id);
			window.location.hash = data.node.a_attr.href;
			document.title =  data.node.id;
			var api = data.node.id.startsWith("/lib/adsk/cm/1.0/sys/") ? '/cs2/rt/types?' : '/cs2/rt/objects?p=';
			$.get(api + data.node.id, function (d) {
				d = JSON.parse(d);
				show_object_list(d, false);
			});
		}
	});
};


var hide_pannel = function(){
	$(".asset_detail").hide();
	$(".asset_table").hide();
	$('#json_view').hide();
	$('#assetDetailTable_parent').hide();
	$('#assetTable_parent').hide();
	$('#save_modification').hide();
}
var show_json = function(path){
	$.get(path, function(data){
		hide_pannel();
		var json_view = $('#json_view');
		json_view.html('<pre ><code class="language-js">' + JSON.stringify(data, null, '  ') + '</code></pre>');

		json_view.show();
	}, "json");
}

// Global data stored for later use.
var gAssetMetaData;
var gIdArray = new Array();
var gDataModifiedById = new Array();

Array.prototype.contains = function (element){

	for (var i = 0; i < this.length; i++){
	if (this[i] == element){
		return true;
	}
	}
	return false;
}

var show_object_list = function(data, isSearch){
	console.log("list objects: ", data.arg);
	hide_pannel();
	gAssetMetaData = data;

	var ns_path = data.arg + "/";
	var table_data = [];
	var len = data.result.length;
	var is_type = ns_path.startsWith("/lib/adsk/cm/1.0/sys/");
	if (is_type){
		for (var asset_name in data.result){
			var asset = data.result[asset_name];
			var description = asset.static_data;
			description = (description && description.assettype) ? description.assettype : "";

			table_data.push(["NO THUMBNAIL", asset_name, "Type", description, asset_name,  ns_path + asset_name]);
		}
	}
	else{
		for (var asset_name in data.result){
			var asset = data.result[asset_name];
			var thumbnail = (asset.thumbnail.length != 0 && asset.thumbnail[0].length > 0) 
				? '<img src="/cs2/fs/file?' + asset.thumbnail[0] + '"/>'
				: "(NO THUMBNAIL)";
			table_data.push([thumbnail, asset.UIName, asset.category.join(',') , asset.description, asset_name,  isSearch ? asset_name : ns_path + asset_name]);

			gIdArray[asset.VersionGUID] = asset_name;
		}
	}

	$('#assetTable').attr("namespace-path", data.arg);
	$("#assetTable").DataTable().clear().rows.add(table_data).draw();

	$("#assetTable tbody").off('click', "tr")
	.on('click', "tr", function(){
		var asset_path =$("#assetTable").DataTable().row(this).data()[5];
		show_obj_detail(asset_path);
	});

	$('#assetTable_parent').show()
}

var apply_returnkey_event = function(keys){
	// Apply a return key event to each cell in the table
	$('#assetTable tbody td').each(function () {
	keys.event.action(this, function (nCell) {
	// Block KeyTable from performing any events while jEditable is in edit mode
	keys.block = true;
	// Initialise the Editable instance for this table
	$(nCell).editable(function (value, settings) {
		// Submit function (local only) - unblock KeyTable
		keys.block = false;
		//var html = this.parentNode.innerHTML;		
		var id = this.parentNode.id;	
		var index = gIdArray[id];
		var className = nCell.className;
		var i = className.indexOf(" focus");
		var propType;
		if (i > 0)
		propType = className.substr(0, i);
		var item = gAssetMetaData.items[index];
		var modified = false;
		if (item.hasOwnProperty(propType))
		{
		if (item[propType] instanceof Array)
		{
			var valueArr = value.split(",");
			if (item[propType] != valueArr){
			modified = true;
			item[propType] = valueArr;
			}
		}
		else{
			if (item[propType] != value){
			modified = true;
			item[propType] = value;
			}
		}
		}
		var itemAfter = gAssetMetaData.items[index];
		if (modified){
		$(this).addClass( 'highlight2' );
		if (!gDataModifiedById.contains(id))
			gDataModifiedById[gDataModifiedById.length] = id;
		}
		return value;
	}, {
		"onblur": 'submit',
		"onreset": function () {
		// Unblock KeyTable, but only after this 'esc' key event has finished. Otherwise
		// it will 'esc' KeyTable as well
			     
		setTimeout(function () { keys.block = false; }, 0);
		}
	});

	// Dispatch click event to go into edit mode - Saf 4 needs a timeout...
	setTimeout(function () { $(nCell).click(); }, 0);
	});
	});

} 

var obj_to_td = function(obj, odd){
	//var text = odd ? '<div class="value odd">' : '<div class="value even">';
	var text = '<div>';
	if ((obj instanceof Array)){
		text += '<table><tbody>';
        for (var i =0; i < obj.length; ++i){
			text += "<tr><td>" + obj_to_td(i, odd) + "</dr>";
			text += "<td>" + obj_to_td(obj[i], odd) + "</td></tr>";
		}
		text += '</tbody></table>';
	}
    else if (typeof obj === 'object'){
        text += '<table><tbody>';
        for (var i in obj){
            text += "<tr><td>" + obj_to_td(i, odd) + "</dr>";
            text += "<td>" + obj_to_td(obj[i], odd) + "</td></tr>";
        }
        text += '</tbody></table>';
    }
	else{
		if (typeof obj === 'string'){
			var extension = obj.replace(/#.+$/, "").split('.').pop();
			extension = extension? extension.toLowerCase() : "";
			if (obj.indexOf("/lib/adsk/cm/1.0/resource") === 0 && (extension == "jpg" || extension == "png"))
				text += '<img src="/cs2/fs/file?' + obj + '"/>';
			else if (obj.indexOf("/lib/adsk/cm/1.0") === 0)
			{
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

var show_obj_detail = function(asset_path){
	var api = asset_path.startsWith("/lib/adsk/cm/1.0/sys/") ? '/cs2/rt/type?' : '/cs2/rt/object?';
	$.get(api + asset_path, function(data){
		console.log("show_obj_detail: ", asset_path);
		$('#assetDetailTable').dataTable().fnDestroy();
		hide_pannel();
		window.location.href = "#o=" + asset_path.replace('#', "%23" );
		var asset_memo = $("#assetDetailTable_parent");

		var text = '<table id="assetDetailTable">'
			+ '<thead><tr><th>Property</th><th>Value</th></tr></thead><tbody>';
	    var odd = false;
		data = data.result;
		for (var i in data){
			if (i === "layered_bottom_f0"){
				odd =odd;
			}
			text += odd ? '<tr class="odd">' : '<tr class="even">' 
			text += '<td>' + i + '</td><td>'
				+ obj_to_td(data[i], odd) +
				'</td></tr>';
			odd = !odd;
		}


		text += '</tbody></table>';

		asset_memo.html(text);

		var lastIdx = null;
		var table = $('#assetDetailTable').dataTable();
		/* keyTableAssetDetail  = new KeyTable({
		   "table": document.getElementById('assetDetailTable')
		   , "datatable": table
		   });
		   apply_assetdetail_keyevent(keyTableAssetDetail);
		$('#assetDetailTable tbody')
			.on( 'mouseleave', function () {
				$( table.cells().nodes() ).removeClass( 'highlight' );
				//$( table.cells().nodes() ).removeClass( 'clicked' );
			} )
		   */

		$("#assetDetailTable").on('click', ".link", function(){
			var obj_path = $(this).text();
			var obj_name = obj_path.replace(/^.+\//, "");
			var ns_path = obj_path.substr(0, obj_path.length - obj_name.length - 1);
			var jst = $('#schema_list').jstree();
			jst.deselect_all();
			jst.select_node(ns_path, true);
			show_obj_detail(obj_path);
			console.log("click link: ", obj_path);
		});
		$('#assetDetailTable_parent').attr("obj-or-type-path", asset_path);
		$("#assetDetailTable_parent").show();

	},"json");

}

var apply_assetdetail_keyevent = function(keys){
	// Apply a return key event to each cell in the table
	$('#assetDetailTable tbody td').each(function () {
		keys.event.action(this, function (nCell) {
			// Block KeyTable from performing any events while jEditable is in edit mode
			keys.block = true;
			// Initialise the Editable instance for this table
			$(nCell).editable(function (value, settings) {
				// Submit function (local only) - unblock KeyTable
				keys.block = false;
            return value;
        }, {
            "onblur": 'submit',
            "onreset": function () {
                // Unblock KeyTable, but only after this 'esc' key event has finished. Otherwise
                // it will 'esc' KeyTable as well
                     
                setTimeout(function () { keys.block = false; }, 0);
            }
        });

        // Dispatch click event to go into edit mode - Saf 4 needs a timeout...
        setTimeout(function () { $(nCell).click(); }, 0);
        });
    });

} 

var save_modification = function(){
    for (var i = 0; i < gDataModifiedById.length; i++)
    {
        var element = gDataModifiedById[i];
        var index = gIdArray[element];
        var item = gAssetMetaData.items[index];
        var js_str = JSON.stringify(item, null, ' ');
        var data_info = '/cm' + gAssetMetaData.path + '/metadata/' + item.VersionGUID + "?Put" + "&Json:" + js_str;
        $.post(data_info, function(data){
        });
    }
    var table = $(".asset_table");
    $( table.find("td")).removeClass('highlight2');
    gDataModifiedById = [];
}

var material_search_submit = function(){
		var keyword = $('#material_search_keyword').val();
		var api = '/cs2/search/objects?p=';
			$.get(api + keyword, function (d) {
				d = JSON.parse(d);
				show_object_list(d, true);
			});
}
