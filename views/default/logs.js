<script>
var updateAll = true;
var lastLogId = 0;

jQuery(document).ready(function() {
    update();
    
    $("#searchInput").on("keyup", function() {
      var value = $(this).val().toLowerCase();
      $("#logsTable tr").filter(function() {
        $(this).toggle($(this).text().toLowerCase().indexOf(value) > -1)
      });
    });
});

function update() {
    if (!updateAll) {
        return;
    }
    
    $.getJSON('logs.js?log=debug&startid=' + lastLogId).done(function(result) {
        if (result.success) {
            if (lastLogId == 0) {
                $('#logsTable').html("");
                setInterval(update, 1000);
            }
            
            $.each(result.messages, function(key, val) {
                if (val.type === "critical") {
                    $('#logsTable').prepend('<tr class="danger"><td>'+val.date+'</td><td>'+val.message+'</td></tr>')
                } else if (val.type === "warning") {
                    $('#logsTable').prepend('<tr class="warning"><td>'+val.date+'</td><td>'+val.message+'</td></tr>')
                } else {
                    var msg = val.message.replace("uFFFD\\uFFFD\\uFFFD\\uFFFD\\uFFFD\\uFFFD\\uFFFD\\uFFFD\\uFFFD\\uFFFD\\uFFFD\\uFFFD\\uFFFD\\uFFFD\\", "...")
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
            updateAll = false;
            alert_error(result.msg);
        }
    }).fail(function(jqxhr, textStatus, error) {
        updateAll = false;
        alert_error("Request Failed: " + error);
    });
}

$('#buttons_placeholder').on('click', '.clear_logs', function(){
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

$('#buttons_placeholder').on('click', '.stop_logs', function(){
	if (updateAll == true) {
		updateAll = false;
		$('.stop_logs').text("Continue");
	} else {
		updateAll = true;
		$('.stop_logs').text("Stop");
	}
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
