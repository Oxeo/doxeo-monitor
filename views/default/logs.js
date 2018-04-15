<script>
var updateInterval = true;
var lastLogId = 0;
var listCmd = [];
var listCmdPosition = -1;
var day = null;

jQuery(document).ready(function() {
    $('#button_newer').hide();
    day = moment();
    updateLogs();
    updateCmdList();
    
    $("#searchInput").on("keyup", function() {
      var value = $(this).val().toLowerCase();
      $("#logsTable tr").filter(function() {
        $(this).toggle($(this).text().toLowerCase().indexOf(value) > -1)
      });
    });
    
    setInterval(updateLogs, 3000);
});

function updateLogs(force = false) {
    if (!updateInterval && force == false) {
        return;
    }
    
    $.getJSON('logs.js?log=debug&startid=' + lastLogId + '&day=' + day.format('YYYY-MM-DD')).done(function(result) {
        if (result.success) {
            if (lastLogId == 0) {
                $('#logsTable').html("");
            }
            
            $.each(result.messages, function(key, val) {
                if (val.type === "critical") {
                    $('#logsTable').prepend('<tr class="danger"><td>'+val.date+'</td><td>'+val.message+'</td></tr>')
                } else if (val.type === "warning") {
                    $('#logsTable').prepend('<tr class="warning"><td>'+val.date+'</td><td>'+val.message+'</td></tr>')
                } else {
                    var msg = val.message;
                    $('#logsTable').prepend('<tr><td>'+val.date+'</td><td>'+msg+'</td></tr>')
                }
                lastLogId = val.id + 1;
            });
            
            // filter
            var value = $("#searchInput").val().toLowerCase();
            $("#logsTable tr").filter(function() {
                $(this).toggle($(this).text().toLowerCase().indexOf(value) > -1)
            });
        } else {
            updateInterval = false;
            $('#stop_logs').text("Continue");
            alert_error(result.msg);
        }
    }).fail(function(jqxhr, textStatus, error) {
        updateInterval = false;
        $('#stop_logs').text("Continue");
        alert_error("Request Failed: " + error);
    });
}

function updateCmdList() {
    $.getJSON('/script/cmd_list.js').done(function(result) {
        listCmd = [];
        $('#cmdlist').empty();
        var regex = new RegExp('"', 'g');
        
        result.records.sort(function(a, b) {
            return Number(b.id) - Number(a.id);
        })
        
        $.each(result.records, function(key, val) {
            listCmd.push(val.cmd);
        });
        
        listCmdPosition = -1;
    }).fail(function(jqxhr, textStatus, error) {
        alert_error("Request Failed: " + error);
    });
}

$('a[href="#older"]').click(function(){
    day.subtract(1, 'day');
    lastLogId = 0;
    $('#logsTable').html('<td colspan="2" class="text-center"><img src="./assets/images/spinner.gif" alt="wait"/></td>');
    updateLogs(true);
    $('#button_newer').show();
    
    updateInterval = false;
    $('#stop_logs').text("Continue");
}); 

$('a[href="#newer"]').click(function(){
    day.add(1, 'day');
    lastLogId = 0;
    $('#logsTable').html('<td colspan="2" class="text-center"><img src="./assets/images/spinner.gif" alt="wait"/></td>');
    updateLogs(true);
    
    if (moment().diff(day, 'days') == 0) {
        $('#button_newer').hide();
        
        updateInterval = true;
        $('#stop_logs').text("Stop");
    }
}); 

$('#clear_logs').click(function(event){
    var data = $(this).data();
    
    param = {
        type: data.clear,
        id: 0
    };
	
	$('#logsTable').text("");
    
    $.getJSON('clear_logs.js', param)
        .done(function(result) {
            if (result.success) {
                //history.back();
            } else {
                alert_error(result.msg);
            }
        }).fail(function(jqxhr, textStatus, error) {
            alert_error("Request Failed: " + error);
    });
});

$('#stop_logs').click(function(event){
	if (updateInterval == true) {
		updateInterval = false;
		$('#stop_logs').text("Continue");
	} else {
		updateInterval = true;
		$('#stop_logs').text("Stop");
	}
});

$('#send').click(function(event) {
    var cmd = $('#sendContent').val();
    sendCmd(cmd);
});

$('#sendContent').on('keyup', function (e) {
    if(e.which === 13) { // enter key
        var cmd = $('#sendContent').val();
        sendCmd(cmd);
    } else if (e.which === 38) { // top key
        previousCmd();
    } else if (e.which === 40) { // down key
        nextCmd();
    }
});

$('#previousCmd').click(function(event){
    previousCmd();
});

$('#nextCmd').click(function(event){
    nextCmd();
});

function previousCmd() {
    if (listCmdPosition+1 < listCmd.length) {
        listCmdPosition++;
        $('#sendContent').val(listCmd[listCmdPosition]);
    }
}

function nextCmd() {
    if (listCmdPosition >= 0) {
        listCmdPosition--;
        if (listCmdPosition == -1) {
            $('#sendContent').val("");
        } else {
            $('#sendContent').val(listCmd[listCmdPosition]);
        }
    }
}

function sendCmd(cmdToSend) {
    var param = {
        cmd: cmdToSend
    };
    
    $('#logsTable').prepend('<tr class="active"><td></td><td>'+cmdToSend+'</td></tr>')
    
    $.getJSON('/script/execute_cmd.js', param)
        .done(function(result) {
            if (result.success) {
                $('#sendContent').val(result.msg);
                updateCmdList();
            } else {
                alert_error(result.msg);
            }
        }).fail(function(jqxhr, textStatus, error) {
            alert_error("Request Failed: " + error);
    });
}

$('#buildBoardCmd').click(function(event){
    var cmd = $('#sendContent').val();
    $('#sendContent').val('helper.sendCmd("' + cmd + '")');
});

$('#clearSearch').click(function(event){
    $('#searchInput').val("");
});

$('#clearCmdContent').click(function(event){
    $('#sendContent').val("");
    listCmdPosition = -1;
});

function alert_error(message) {           
    $('#alert_placeholder').prepend(
        '<div style="display: none;" class="alert alert-danger fade in" role="alert">' +
            '<strong>Error!</strong> '+message+
            '<button type="button" class="close" data-dismiss="alert" aria-label="Close">' +
                '<span aria-hidden="true">&times;</span>' +
            '</button>'+
        '</div>'
    );
    
    $('.alert-danger').slideDown( "fast" );
}

</script>

<script src="/assets/fullcalendar/moment.min.js" type="text/javascript"></script>
