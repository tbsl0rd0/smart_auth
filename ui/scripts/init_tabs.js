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
$('#auth_status_tab').on('click', function() {
  $('#current_auth_stack_items').text('');
  $('#current_auth_stack_items_2').text('');
  $('#available_auth_stack_items').text('');
  set_items();
});
