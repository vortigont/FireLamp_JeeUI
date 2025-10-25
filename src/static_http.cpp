#ifndef NO_EMBEDDED_FILES

#include "EmbUI.h"
#include "../embed/embed.h"


enum class embedded_data_t {
  index,
  ui,
  ui_i18n,
  firelamp,
  lodash,
  embuijs,
  embuijson,
  embuii18n,
  embuilang,
  tz
};

static constexpr const char INDEX_FILE[] = "/index.html";
static constexpr const char C_TYPE_JS_UTF8[] = "application_javascript; charset=utf-8";

static void mk_response(embedded_data_t obj, AsyncWebServerRequest* req){
  AsyncWebServerResponse* response;
  if (req->hasHeader(asyncsrv::T_INM) && req->header(asyncsrv::T_INM).equals(__BUILD_TSTAMP)) {
    response = new AsyncBasicResponse(304);  // Not modified
  } else {
    switch (obj){
      case embedded_data_t::index :
        response = req->beginResponse(200, asyncsrv::T_text_html, index_html_gz_start, index_html_gz_end - index_html_gz_start );
        break;
      case embedded_data_t::ui :
        response = req->beginResponse(200, asyncsrv::T_application_json, ui_json_gz_start, ui_json_gz_end - ui_json_gz_start );
        break;
      case embedded_data_t::ui_i18n :
        response = req->beginResponse(200, asyncsrv::T_application_json, ui_i18n_json_gz_start, ui_i18n_json_gz_end - ui_i18n_json_gz_start );
        break;
      case embedded_data_t::firelamp :
        response = req->beginResponse(200, C_TYPE_JS_UTF8, firelamp_js_gz_start, firelamp_js_gz_end - firelamp_js_gz_start );
        break;
      case embedded_data_t::lodash :
        response = req->beginResponse(200, C_TYPE_JS_UTF8, lodash_js_gz_start, lodash_js_gz_end - lodash_js_gz_start );
        break;
      case embedded_data_t::embuijs :
        response = req->beginResponse(200, C_TYPE_JS_UTF8, embui_js_gz_start, embui_js_gz_end - embui_js_gz_start );
        break;
      case embedded_data_t::embuijson :
        response = req->beginResponse(200, asyncsrv::T_application_json, ui_embui_json_gz_start, ui_embui_json_gz_end - ui_embui_json_gz_start );
        break;
      case embedded_data_t::embuii18n :
        response = req->beginResponse(200, asyncsrv::T_application_json, ui_embui_i18n_json_gz_start, ui_embui_i18n_json_gz_end - ui_embui_i18n_json_gz_start );
        break;
      case embedded_data_t::embuilang :
        response = req->beginResponse(200, asyncsrv::T_application_json, ui_embui_lang_json_gz_start, ui_embui_lang_json_gz_end - ui_embui_lang_json_gz_start );
        break;
      case embedded_data_t::tz :
        response = req->beginResponse(200, asyncsrv::T_application_json, tz_json_gz_start, tz_json_gz_end - tz_json_gz_start );
        break;
      default :
        response = new AsyncBasicResponse(404);
        return req->send(response);
    }
  }
  response->addHeader(asyncsrv::T_Content_Encoding, asyncsrv::T_gzip);
  response->addHeader(asyncsrv::T_ETag, __BUILD_TSTAMP);
  req->send(response);
}


void set_static_http_handlers(){
  embui.server.rewrite("/", INDEX_FILE);
  embui.server.on("/index.html", HTTP_GET, [](AsyncWebServerRequest *request){ mk_response(embedded_data_t::index, request); } );
  embui.server.on("/js/firelamp.js", HTTP_GET, [](AsyncWebServerRequest *request){ mk_response(embedded_data_t::firelamp, request); } );
  embui.server.on("/js/ui.json", HTTP_GET, [](AsyncWebServerRequest *request){ mk_response(embedded_data_t::ui, request); } );
  embui.server.on("/js/ui.i18n.json", HTTP_GET, [](AsyncWebServerRequest *request){ mk_response(embedded_data_t::ui_i18n, request); } );
  // embui
  embui.server.on("/js/lodash.js", HTTP_GET, [](AsyncWebServerRequest *request){ mk_response(embedded_data_t::lodash, request); } );
  embui.server.on("/js/embui.js", HTTP_GET, [](AsyncWebServerRequest *request){ mk_response(embedded_data_t::embuijs, request); } );
  embui.server.on("/js/ui_embui.json", HTTP_GET, [](AsyncWebServerRequest *request){ mk_response(embedded_data_t::embuijson, request); } );
  embui.server.on("/js/ui_embui.i18n.json", HTTP_GET, [](AsyncWebServerRequest *request){ mk_response(embedded_data_t::embuii18n, request); } );
  embui.server.on("/js/ui_embui.lang.json", HTTP_GET, [](AsyncWebServerRequest *request){ mk_response(embedded_data_t::embuilang, request); } );
  embui.server.on("/js/tz.json", HTTP_GET, [](AsyncWebServerRequest *request){ mk_response(embedded_data_t::tz, request); } );

}

#endif  // NO_EMBEDDED_FILES