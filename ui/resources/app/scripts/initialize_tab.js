$('#tab > .item').tab();

$('.animated_tada').on('mouseover', () => {
  $(this).addClass('animated tada');
  setTimeout(() => {
    $(this).removeClass('animated tada');
  }, 800);
});

$('.animated_tada').on('mouseout', () => {
  $(this).removeClass('animated tada');
});
