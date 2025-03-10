#ifndef NO_EMBEDDED_FILES

#include "EmbUI.h"
#include "../embed/embed.h"


enum class embedded_data_t {
  index,
  ui,
  i18n,
  script
};

static constexpr const char INDEX_FILE[] = "/index.html";

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
      case embedded_data_t::i18n :
        response = req->beginResponse(200, asyncsrv::T_application_json, ui_i18n_json_gz_start, ui_i18n_json_gz_end - ui_i18n_json_gz_start );
        break;
      case embedded_data_t::script :
        response = req->beginResponse(200, asyncsrv::T_application_javascript, script_js_gz_start, script_js_gz_end - script_js_gz_start );
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
  embui.server.on("/script.js", HTTP_GET, [](AsyncWebServerRequest *request){ mk_response(embedded_data_t::script, request); } );
  embui.server.on("/ui.json", HTTP_GET, [](AsyncWebServerRequest *request){ mk_response(embedded_data_t::ui, request); } );
  embui.server.on("/i18n.json", HTTP_GET, [](AsyncWebServerRequest *request){ mk_response(embedded_data_t::i18n, request); } );
}

#endif  // NO_EMBEDDED_FILES