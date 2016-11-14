$('#tab > .item').tab();

$('.animated_tada').on('mouseover', function() {
  $(this).addClass('animated tada');

  setTimeout(() => {
    $(this).removeClass('animated tada');
  }, 800);
});

$('.animated_tada').on('mouseout', function() {
  $(this).removeClass('animated tada');
});
