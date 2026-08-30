#ifndef NO_EMBEDDED_FILES

#include "EmbUI.h"
#include "../embed/embed.h"


enum class embedded_data_t {
  // html
  index,
  // embui
  lodash,
  embuijs,
  ui_embui,
  ui_embuii18n,
  ui_embuilang,
  tz,
  // js
  informer,
  ui,
  ui_i18n,
  // css
  pure,
  style,
  style_dark,
  style_light,
  // images
  menu,
  menu_dark,
  menu_light,
  wp_dark,
  wp_light,
  android_chrome_192x192,
  android_chrome_512x512,
  end
};

static constexpr const char INDEX_FILE[] = "/index.html";

static void mk_response(embedded_data_t obj, AsyncWebServerRequest* req){
  AsyncWebServerResponse* response;
  if (req->hasHeader(asyncsrv::T_INM) && req->header(asyncsrv::T_INM).equals(__BUILD_TSTAMP)) {
    response = new AsyncBasicResponse(304);  // Not modified
  } else {
    switch (obj){
      // Informer
      case embedded_data_t::index :
        response = req->beginResponse(200, asyncsrv::T_text_html, embed_index_html_gz_start, embed_index_html_gz_end - embed_index_html_gz_start );
        response->addHeader(asyncsrv::T_Content_Encoding, asyncsrv::T_gzip);
        break;
      case embedded_data_t::ui :
        response = req->beginResponse(200, asyncsrv::T_application_json, embed_ui_json_gz_start, embed_ui_json_gz_end - embed_ui_json_gz_start );
        response->addHeader(asyncsrv::T_Content_Encoding, asyncsrv::T_gzip);
        break;
      case embedded_data_t::ui_i18n :
        response = req->beginResponse(200, asyncsrv::T_application_json, embed_ui_i18n_json_gz_start, embed_ui_i18n_json_gz_end - embed_ui_i18n_json_gz_start );
        response->addHeader(asyncsrv::T_Content_Encoding, asyncsrv::T_gzip);
        break;
      case embedded_data_t::informer :
        response = req->beginResponse(200, asyncsrv::T_application_javascript, embed_infrmr_js_gz_start, embed_infrmr_js_gz_end - embed_infrmr_js_gz_start );
        response->addHeader(asyncsrv::T_Content_Encoding, asyncsrv::T_gzip);
        break;
      // libs
      case embedded_data_t::lodash :
        response = req->beginResponse(200, asyncsrv::T_application_javascript, jsdir_lodash_js_gz_start, jsdir_lodash_js_gz_end - jsdir_lodash_js_gz_start );
        response->addHeader(asyncsrv::T_Content_Encoding, asyncsrv::T_gzip);
        break;
      case embedded_data_t::embuijs :
        response = req->beginResponse(200, asyncsrv::T_application_javascript, jsdir_embui_js_gz_start, jsdir_embui_js_gz_end - jsdir_embui_js_gz_start );
        response->addHeader(asyncsrv::T_Content_Encoding, asyncsrv::T_gzip);
        break;
      case embedded_data_t::ui_embui :
        response = req->beginResponse(200, asyncsrv::T_application_json, jsdir_ui_embui_json_gz_start, jsdir_ui_embui_json_gz_end - jsdir_ui_embui_json_gz_start );
        response->addHeader(asyncsrv::T_Content_Encoding, asyncsrv::T_gzip);
        break;
      case embedded_data_t::ui_embuii18n :
        response = req->beginResponse(200, asyncsrv::T_application_json, jsdir_ui_embui_i18n_json_gz_start, jsdir_ui_embui_i18n_json_gz_end - jsdir_ui_embui_i18n_json_gz_start );
        response->addHeader(asyncsrv::T_Content_Encoding, asyncsrv::T_gzip);
        break;
      case embedded_data_t::ui_embuilang :
        response = req->beginResponse(200, asyncsrv::T_application_json, jsdir_ui_embui_lang_json_gz_start, jsdir_ui_embui_lang_json_gz_end - jsdir_ui_embui_lang_json_gz_start );
        response->addHeader(asyncsrv::T_Content_Encoding, asyncsrv::T_gzip);
        break;
      case embedded_data_t::tz :
        response = req->beginResponse(200, asyncsrv::T_application_json, jsdir_tz_json_gz_start, jsdir_tz_json_gz_end - jsdir_tz_json_gz_start );
        response->addHeader(asyncsrv::T_Content_Encoding, asyncsrv::T_gzip);
        break;
      // css
      case embedded_data_t::pure :
        response = req->beginResponse(200, asyncsrv::T_text_css, cssdir_pure_css_gz_start, cssdir_pure_css_gz_end - cssdir_pure_css_gz_start );
        response->addHeader(asyncsrv::T_Content_Encoding, asyncsrv::T_gzip);
        break;
      case embedded_data_t::style :
        response = req->beginResponse(200, asyncsrv::T_text_css, embed_style_css_gz_start, embed_style_css_gz_end - embed_style_css_gz_start );
        response->addHeader(asyncsrv::T_Content_Encoding, asyncsrv::T_gzip);
        break;
      case embedded_data_t::style_dark :
        response = req->beginResponse(200, asyncsrv::T_text_css, embed_style_dark_css_gz_start, embed_style_dark_css_gz_end - embed_style_dark_css_gz_start );
        response->addHeader(asyncsrv::T_Content_Encoding, asyncsrv::T_gzip);
        break;
      case embedded_data_t::style_light :
        response = req->beginResponse(200, asyncsrv::T_text_css, embed_style_light_css_gz_start, embed_style_light_css_gz_end - embed_style_light_css_gz_start );
        response->addHeader(asyncsrv::T_Content_Encoding, asyncsrv::T_gzip);
        break;
      // pics
      case embedded_data_t::menu :
        response = req->beginResponse(200, asyncsrv::T_image_jpeg, cssdir_menu_jpg_start, cssdir_menu_jpg_end - cssdir_menu_jpg_start );
        break;
      case embedded_data_t::menu_dark :
        response = req->beginResponse(200, asyncsrv::T_image_webp, cssdir_menu_dark_webp_start, cssdir_menu_dark_webp_end - cssdir_menu_dark_webp_start );
        break;
      case embedded_data_t::menu_light :
        response = req->beginResponse(200, asyncsrv::T_image_webp, cssdir_menu_light_webp_start, cssdir_menu_light_webp_end - cssdir_menu_light_webp_start );
        break;
      case embedded_data_t::wp_dark :
        response = req->beginResponse(200, asyncsrv::T_image_webp, cssdir_wp_dark_svg_gz_start, cssdir_wp_dark_svg_gz_end - cssdir_wp_dark_svg_gz_start );
        break;
      case embedded_data_t::wp_light :
        response = req->beginResponse(200, asyncsrv::T_image_webp, cssdir_wp_light_svg_gz_start, cssdir_wp_light_svg_gz_end - cssdir_wp_light_svg_gz_start );
        break;
      case embedded_data_t::android_chrome_192x192 :
        response = req->beginResponse(200, asyncsrv::T_image_webp, cssdir_android_chrome_192x192_webp_start, cssdir_android_chrome_192x192_webp_end - cssdir_android_chrome_192x192_webp_start );
        break;
      case embedded_data_t::android_chrome_512x512 :
        response = req->beginResponse(200, asyncsrv::T_image_webp, cssdir_android_chrome_512x512_webp_start, cssdir_android_chrome_512x512_webp_end - cssdir_android_chrome_512x512_webp_start );
        break;
//      case embedded_data_t:: :
//        response = req->beginResponse(200, asyncsrv::T_application_json, _start, _end - _start );
//        break;


      default :
        response = new AsyncBasicResponse(404);
        return req->send(response);
    }
  }
  response->addHeader(asyncsrv::T_ETag, __BUILD_TSTAMP);
  req->send(response);
}


void set_static_http_handlers(){
  embui.server.rewrite("/", INDEX_FILE);
  embui.server.on(INDEX_FILE, HTTP_GET, [](AsyncWebServerRequest *request){ mk_response(embedded_data_t::index, request); } );
  // embui
  embui.server.on("/js/lodash.js", HTTP_GET, [](AsyncWebServerRequest *request){ mk_response(embedded_data_t::lodash, request); } );
  embui.server.on("/js/embui.js", HTTP_GET, [](AsyncWebServerRequest *request){ mk_response(embedded_data_t::embuijs, request); } );
  embui.server.on("/js/ui_embui.json", HTTP_GET, [](AsyncWebServerRequest *request){ mk_response(embedded_data_t::ui_embui, request); } );
  embui.server.on("/js/ui_embui.i18n.json", HTTP_GET, [](AsyncWebServerRequest *request){ mk_response(embedded_data_t::ui_embuii18n, request); } );
  embui.server.on("/js/ui_embui.lang.json", HTTP_GET, [](AsyncWebServerRequest *request){ mk_response(embedded_data_t::ui_embuilang, request); } );
  embui.server.on("/js/tz.json", HTTP_GET, [](AsyncWebServerRequest *request){ mk_response(embedded_data_t::tz, request); } );
  // informer js
  embui.server.on("/js/infrmr.js", HTTP_GET, [](AsyncWebServerRequest *request){ mk_response(embedded_data_t::informer, request); } );
  embui.server.on("/js/ui.json", HTTP_GET, [](AsyncWebServerRequest *request){ mk_response(embedded_data_t::ui, request); } );
  embui.server.on("/js/ui.i18n.json", HTTP_GET, [](AsyncWebServerRequest *request){ mk_response(embedded_data_t::ui_i18n, request); } );
  // css
  embui.server.on("/css/pure.css", HTTP_GET, [](AsyncWebServerRequest *request){ mk_response(embedded_data_t::pure, request); } );
  embui.server.on("/css/style.css", HTTP_GET, [](AsyncWebServerRequest *request){ mk_response(embedded_data_t::style, request); } );
  embui.server.on("/css/style_dark.css", HTTP_GET, [](AsyncWebServerRequest *request){ mk_response(embedded_data_t::style_dark, request); } );
  embui.server.on("/css/style_light.css", HTTP_GET, [](AsyncWebServerRequest *request){ mk_response(embedded_data_t::style_light, request); } );
  // img
  embui.server.on("/css/menu.css", HTTP_GET, [](AsyncWebServerRequest *request){ mk_response(embedded_data_t::menu, request); } );
  embui.server.on("/css/menu_dark.css", HTTP_GET, [](AsyncWebServerRequest *request){ mk_response(embedded_data_t::menu_dark, request); } );
  embui.server.on("/css/menu_light.css", HTTP_GET, [](AsyncWebServerRequest *request){ mk_response(embedded_data_t::menu_light, request); } );
  embui.server.on("/css/wp_dark.css", HTTP_GET, [](AsyncWebServerRequest *request){ mk_response(embedded_data_t::wp_dark, request); } );
  embui.server.on("/css/wp_light.css", HTTP_GET, [](AsyncWebServerRequest *request){ mk_response(embedded_data_t::wp_light, request); } );
  embui.server.on("/css/android_chrome_192x192.webp", HTTP_GET, [](AsyncWebServerRequest *request){ mk_response(embedded_data_t::android_chrome_192x192, request); } );
  embui.server.on("/css/android_chrome_512x512.webp", HTTP_GET, [](AsyncWebServerRequest *request){ mk_response(embedded_data_t::android_chrome_512x512, request); } );
  
}

#endif  // NO_EMBEDDED_FILES