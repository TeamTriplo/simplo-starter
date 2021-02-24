jQuery(document).ready(function($) {
  $( document ).ready(function() {
    if (($(".node-type-crowdfunding-offer .field-name-field-hero-image") && $(".l-page-title .page-title")).length) {
      $(".hero_image_wrapper").insertAfter("header.l-header");
      $("h1.page-title").appendTo(".hero_image_wrapper .hero_page_title .container");
    }
    
    $('.sppx-session-logout').click(function(){
      var confirmed = confirm("Do you really want to log out?");
      if(confirmed){
        // AJAX POST request to SPPX 
        $.ajax({
          url: "/sppx-logout",
          type: "post",
          success: function (response) {
            let result = JSON.parse(response);
            response_code =  result['status']['code'];
            if(response_code != '200'){
              alert(result['status']['memo']);
            }else{
              location.href = location.href
            }
          },
          error: function(jqXHR, textStatus, errorThrown) {
            console.log(textStatus, errorThrown);
          }
        });
      }
      return false;
    });

  });
});