jQuery(document).ready(function($) {
  $( document ).ready(function() {
    if (($(".node-type-crowdfunding-offer .field-name-field-hero-image") && $(".l-page-title .page-title")).length) {
      $(".hero_image_wrapper").insertAfter("header.l-header");
      $("h1.page-title").appendTo(".hero_image_wrapper .hero_page_title .container");
    }
  });
});
