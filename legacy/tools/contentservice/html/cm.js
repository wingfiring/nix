$(document).ready(function(){
	$.ajaxSetup({cache: true});
	//show_schema_list_v1();
	show_schema_list();
	$('#save_modification').hide();
});

var keyTable;
var keyTableAssetDetail;

var onschema_click = function(e){
	e = e||event;
	if (e && e.preventDefault)
		e.preventDefault();

	var target = e.target;
	if (target.tagName == 'A') {
		if (target.className=='type')
			show_json(e.target.href);
		else{
			//show_object_list_v1(e.target.href);
			//show_object_list_sample(e.target.href);
			show_object_list(e.target.href);
		}
	}
}

var show_schema_list_v1 = function(){
	$.get("/cm/lib/adsk/cm/type?List", function(data, textStates){
		var len = data.items.length;
		var schema_list = $("#schema_list");
		schema_list.hide();
		var i;
		for (i = 0; i < len; ++i){
			var node = "<p><span><a class=\"assetlist\" href=\"/cm/lib/adsk/cm/asset/" + data.items[i] + "?Get" + "\">" + data.items[i] +  "</a> </span>"
					+   "<span>(<a class=\"type\" href=\"/cm/lib/adsk/cm/type/" + data.items[i] + "?Get" + "\">JSON</a>) </span></p>";
			schema_list.append($(node));
		}
		schema_list.show();
	}, "json");
}

var show_schema_list = function(){
	$.get("/cm/lib/adsk/cm/type?List", function(data, textStates){
		var len = data.items.length;
	var array = ["Ceramic", "Concrete", "Generic", "Glazing", "Hardwood", "MasonryCMU", "Metal",
		"MetallicPaint", "PlasticVinyl", "PrismLayered", "PrismMetal", "PrismOpaque", "PrismTransparent",
		"SolidGlass", "Stone", "WallPaint"];
	var schema_tree = $("#schema_list");
	schema_tree.hide();		

	var ul = $('<ul>');
	//$('ul li').remove();
	var i;
	for (i = 0; i < len; ++i){	
		var element = data.items[i];
		var index = element.indexOf("Schema");
		var name = "";
		if (index > 0)
		name = element.substr(0, index);
		if ($.inArray(name, array) != -1){
		//var node = "<li" +  " id=\"" + element + "\"" + " class=\"assetlist\""  
		//+ " href=\"/cm/lib/adsk/cm/asset/" + element + "?Get"  + "\">" +  element + "</li>";
		// Add a link <a> so that the href can be well defined and easily captured by onschema_click function.
		var node = "<li" +  " id=\"" + element + "\"" + ">" + "<a" + " class=\"assetlist\""  
		+ "href=\"/cm/lib/adsk/cm/asset/" + element + "?Get"  + "\">" +  element + "</li>";
		ul.append($(node));
		}
	}
	ul.appendTo(schema_tree);
	schema_tree.show();
	schema_tree.jstree();
	}, "json");
}

var hide_pannel = function(){
	$(".asset_detail").hide();
	$(".asset_table").hide();
	$('#json_view').hide();
}
var show_json = function(path){
	$.get(path, function(data){
		hide_pannel();
		var json_view = $('#json_view');
		json_view.html('<pre ><code class="language-js">' + JSON.stringify(data, null, '  ') + '</code></pre>');

		json_view.show();
	}, "json");
}

var show_object_list_v1 = function(path){
	$.get(path, function(data){
		hide_pannel();

		var asset_table = $(".asset_table");

		var len = data.items.length;
		var text = '<div id="schema_path">' + data.path +'</div><table><tbody>'
			+ '<thead><tr><th>Thumbnai</th><th>UIName</th><th>Category</th><th>Description</th><th>IDentifier</th></tr></thead>';
		for (var i = 0; i < len; ++i){
			var asset = data.items[i];
			if (asset.thumbnail.length != 0 && asset.thumbnail[0].length > 0){
				text += '<tr><td> <img src="' + asset.thumbnail[0] + "?Get" + '"/></td>';
			}else{
				text += '<tr><td> NO THUMBNAIL('+ asset.thumbnail.length +') </td>';
			}
			text += '<td>' + asset.UIName + '</td>'
				+ '<td>' + asset.category + '</td>'
				+ '<td>' + asset.description + '</td>'
				+ '<td>' + data.assets[i] + '</td>'
				'</tr>';
		}

		text += '</tbody></table>';
		asset_table.html(text);

		$(function(){
			$("tr:odd").addClass("odd");
			$("tr:even").addClass("even");
		});

		$(".asset_table tbody>tr").click(function(){
			show_detail(this);}
			);
		asset_table.show();
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

var show_object_list = function(path){
	$.get(path, function(data){
	hide_pannel();
	gAssetMetaData = data;
	var asset_table = $(".asset_table");
	var len = data.items.length;
	var text = '<div id="schema_path">' + data.path + '</div>' + '<table id="assetTable" class="row-border hover order-column" cellspacing="0" width="100%">';
	text += '<thead><tr><th>Thumbnail</th><th>UIName</th><th>Category</th><th>Description</th><th>IDentifier</th></tr></thead>';
	for (var i = 0; i < len; ++i){
		var asset = data.items[i];
		var id = asset.VersionGUID;
		// Insert 'id' to each row.
		if (asset.thumbnail.length != 0 && asset.thumbnail[0].length > 0){
			text += '<tr id="' + id + '">' + '<td> <img src="' + asset.thumbnail[0] + "?Get" + '"/></td>';
		}
		else{
			text += '<tr id="' + id + '">' + '<td> NO THUMBNAIL(' + asset.thumbnail.length +') </td>';
		}
		text += '<td class="UIName">' + asset.UIName + '</td>'
			+ '<td class="category">' + asset.category + '</td>'
			+ '<td class="description">' + asset.description + '</td>'
			+ '<td>' + data.assets[i] + '</td>'
			+ '</tr>';
		gIdArray[id] = i;
	}

	//text += '</tbody></table>';
	text += '</table>';
	asset_table.html(text);
		
	$(function(){
		var lastIdx = null;
		var table = $('#assetTable')
		.on('page.dt', function(){
			var table_test = $('assetTable').dataTable();
			//var pageInfo = table_test.api.page();
			// When page changes, need to add return key event for new page cells.
			apply_returnkey_event(keyTable);
		}).dataTable();

		keyTable  = new KeyTable({
		"table": document.getElementById('assetTable'),
		"datatable": table
		});


		apply_returnkey_event(keyTable);
		
		$('#assetTable tbody')
		.on( 'mouseover', 'td', function () {
			var colIdx = table.cell(this).index().column;

			if ( colIdx !== lastIdx ) {
				$( table.cells().nodes() ).removeClass( 'highlight' );
				$( table.column( colIdx ).nodes() ).addClass( 'highlight' );
				//$( table.cells().nodes() ).removeClass( 'clicked' );
				//$( table.column( colIdx ).nodes() ).addClass( 'clicked' );
			}
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
			if (item.hasOwnProperty(propType))
			{
			if (item[propType] instanceof Array)
			{
				var valueArr = value.split(",");
				item[propType] = valueArr;
			}
			else
				item[propType] = value;
			}
			var itemAfter = gAssetMetaData.items[index];
			if (!gDataModifiedById.contains(id))
			gDataModifiedById[gDataModifiedById.length] = id;

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
		} )
		.on( 'mouseleave', function () {
			$( table.cells().nodes() ).removeClass( 'highlight' );
			//$( table.cells().nodes() ).removeClass( 'clicked' );
		} )
		//} );
		.on( 'dblclick', 'td', function () {
			$( table.cells().nodes() ).removeClass( 'highlight' );
			$( table.cells().nodes() ).removeClass( 'highlight2' );
			//$(this).removeClass("highlight2");
			$(this).addClass("highlight2");	
		} );
		} );
		// TODO: handle it later. Lee comment out for temp.
		$(".asset_table tbody>tr").dblclick(function(){
		show_detail(this);}
		);
	asset_table.show();
	$('#save_modification').show();
	}, "json");
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

// From original sample of DataTable
var show_object_list_sample = function(path){
	$.get(path, function(data){
	    hide_pannel();

	    var asset_table = $(".asset_table");
	    var text = '<table id="assetTable" class="row-border hover order-column" cellspacing="0" width="100%">';
	    text += '<thead><tr><th>Name</th><th>Position</th><th>Office</th><th>Age</th><th>Start date</th><th>Salary</th></tr></thead>';
	    text += '<tbody><tr><td>Tiger Nixon</td><td>System Architect</td><td>Edinburgh</td><td>61</td><td>2011/04/25</td><td>$320,800</td></tr><tr><td>Garrett Winters</td><td>Accountant</td><td>Tokyo</td><td>63</td><td>2011/07/25</td><td>$170,750</td></tr></tbody></table>';
	    asset_table.html(text);
	    $(function(){
	    	var lastIdx = null;
   	    	var table = $('#assetTable').DataTable();
	
	 	$('#assetTable tbody')
		    .on( 'mouseover', 'td', function () {
			var colIdx = table.cell(this).index().column;

			if ( colIdx !== lastIdx ) {
				$( table.cells().nodes() ).removeClass( 'highlight' );
				$( table.column( colIdx ).nodes() ).addClass( 'highlight' );
			}
		    } )
		    .on( 'mouseleave', function () {
			$( table.cells().nodes() ).removeClass( 'highlight' );
		    } );
	    } );
	    $(".asset_table tbody>tr").click(function(){
			show_detail(this);}
			);
	    asset_table.show();
	}, "json");
}

var obj_to_td = function(obj, odd){
	//var text = odd ? '<div class="value odd">' : '<div class="value even">';
	var text = '<div>';
	if ((obj instanceof Array) || (typeof obj === 'object')){
		text += '<table><tbody>';
		for (var i in obj){
			text += "<tr><td>" + obj_to_td(i, odd) + "</dr>";
			text += "<td>" + obj_to_td(obj[i], odd) + "</td></tr>";
		}
		text += '</tbody></table>';
	}
	else{
		if (typeof obj === 'string'){
			if (obj.indexOf("/cm/lib/adsk/cm/resource/") === 0)
				text += '<img src="' + obj + '"/>';
			else if (obj.indexOf("/lib/adsk/cm/asset/") === 0)
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

var show_detail = function(e){
	var id = e.cells[e.cells.length - 1].innerHTML;
	var schema_path = $("#schema_path").text();

	var asset_path = "/cm" + schema_path + '/' + id + "?Get";
	//do_show_detail(asset_path);
	do_show_detail_2(asset_path);
}

var do_show_detail = function(asset_path){
	$.get(asset_path, function(data){
		hide_pannel();
		var asset_memo = $("#asset_memo");

		var text = '<table><tbody>'
			+ '<thead><tr><th>Property</th><th>Value</th></tr></thead>';
	    var odd = false;
		for (var i = 0 in data){
			text += odd ? '<tr class="odd">' : '<tr class="even">' 
			text += '<td>' + i + '</td><td>'
				+ obj_to_td(data[i], odd) +
				'</td></tr>';
			odd = !odd;
		}

		text += '</tbody></table>';
		asset_memo.html(text);

		/*$(function(){
			$("tr:odd").addClass("odd");
			$("tr:even").addClass("even");
		});*/

		$(".link").click(function(){do_show_detail('/cm/' +this.innerHTML + "?Get");});
		$(".asset_detail").show();
	},"json");

}

var do_show_detail_2 = function(asset_path){
	$.get(asset_path, function(data){
		hide_pannel();
		var asset_memo = $("#asset_memo");

		var text = '<table id="assetDetailTable">'
			+ '<thead><tr><th>Property</th><th>Value</th></tr></thead>';
	    var odd = false;
		for (var i = 0 in data){
			text += odd ? '<tr class="odd">' : '<tr class="even">' 
			text += '<td>' + i + '</td><td>'
				+ obj_to_td(data[i], odd) +
				'</td></tr>';
			odd = !odd;
		}


		//text += '</tbody></table>';
		text += '</table>';

		asset_memo.html(text);

	    $(function(){
		var lastIdx = null;
		var table = $('#assetDetailTable').dataTable();
		keyTableAssetDetail  = new KeyTable({
			"table": document.getElementById('assetDetailTable')
			, "datatable": table
		    });
		apply_assetdetail_keyevent(keyTableAssetDetail);

	    $('#assetDetailTable tbody')
		.on( 'mouseover', 'td', function () {
			var colIdx = table.cell(this).index().column;

			if ( colIdx !== lastIdx ) {
				$( table.cells().nodes() ).removeClass( 'highlight' );
				$( table.column( colIdx ).nodes() ).addClass( 'highlight' );
				//$( table.cells().nodes() ).removeClass( 'clicked' );
				//$( table.column( colIdx ).nodes() ).addClass( 'clicked' );
			}
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
/*		    var item = gAssetMetaData.items[index];
		    if (item.hasOwnProperty(propType))
		    {
			if (item[propType] instanceof Array)
			{
			    var valueArr = value.split(",");
			    item[propType] = valueArr;
			}
			else
			    item[propType] = value;
		    }
		    var itemAfter = gAssetMetaData.items[index];
		    if (!gDataModifiedById.contains(id))
		        gDataModifiedById[gDataModifiedById.length] = id;
*/
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
		} )
		.on( 'mouseleave', function () {
			$( table.cells().nodes() ).removeClass( 'highlight' );
			//$( table.cells().nodes() ).removeClass( 'clicked' );
		} )

		});
		$(".link").click(function(){do_show_detail_2('/cm/' +this.innerHTML + "?Get");});
		$(".asset_detail").show();
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
/*            //var html = this.parentNode.innerHTML;		
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
*/
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
var show_asset_list = function(){
}

var back_to_list = function(){
	hide_pannel();
	$(".asset_table").show();
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
		//TODO: Why not receive breakpoint here
		//var table = $(".asset_table");
		//$( table.column( index ).nodes() ).removeClass( 'highlight2' );
        });
    }
    var table = $(".asset_table");
    $( table.find("td")).removeClass('highlight2');
    gDataModifiedById = [];

}
