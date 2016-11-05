$('.menu .item').tab();

$('.animated_tada').on('mouseover', function() {
  $(this).addClass('animated tada');
  setTimeout(function() {
    $(this).removeClass('animated tada');
  }, 800);
})

$('.animated_tada').on('mouseout', function() {
  $(this).removeClass('animated tada');
});

$('#authentication_status_tab').on('click', function() {
  $('#current_authentication_factor_list').text('');
  $('#current_authentication_factor_list_2').text('');
  $('#available_authentication_factor_list').text('');
  set_items();
});
