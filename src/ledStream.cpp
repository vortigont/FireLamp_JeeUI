#include "ledStream.h"
#ifdef USE_STREAMING
AsyncWebSocket wsStream("/stream");  // TODO: переделать
Led_Stream *ledStream = nullptr;

void Led_Stream::handleE131Packet(e131_packet_t* p, const IPAddress &clientIP, bool isArtnet) {
    //E1.31 protocol support
    // if (ledStream->getBuff() == nullptr || !myLamp.isLampOn())
    if (!myLamp.isLampOn())
        return;

    uint16_t uni = 0;
    uint8_t* e131_data = nullptr;
    uint8_t seq = 0;

    if (isArtnet) {
        uni = p->art_universe;
        e131_data = p->art_data;
        seq = p->art_sequence_number;
    } else {
        uni = htons(p->universe);
        e131_data = p->property_values;
        seq = p->sequence_number;
    }

    // only listen for universes we're handling & allocated memory
    if (uni >= (ledStream->getUni() + ledStream->getUniCt())) {
        return;
    }

    uint16_t lastUni = uni - ledStream->getUni();
    uint16_t ledsPerUni  = mx.cfg.w() * ledStream->getLineQt();

    ledStream->getLastSeqNum()[uni-ledStream->getUni()] = seq;
    if (lastUni == 0) {
        // first universe of this fixture
        uint16_t count = min(ledsPerUni, _w*_h);
        ledStream->fillLeds((CRGB*)&e131_data[ledStream->getDMXAddress()], count);
    } 
    else if (lastUni > 0 && uni < (ledStream->getUni() + ledStream->getUniCt())) {
        // additional universe(s) of this fixture
        uint16_t lastUniLedQt = ledsPerUni;     // first universe
        if (lastUni > 1) {
            lastUniLedQt += ledsPerUni * (lastUni - 1);  // extended universe(s) before current
        }
        uint16_t remainingLeds = _w*_h - lastUniLedQt;
        uint16_t count = min(ledsPerUni, remainingLeds);
        ledStream->fillLeds((CRGB*)&e131_data[ledStream->getDMXAddress()], count,  lastUniLedQt);
        if (uni == ledStream->getUniCt() && myLamp.isDirect())
            FastLED.show();
    }
}

void Led_Stream::fillLeds(CRGB *ledsPtr, uint16_t ledsQt, uint16_t lastLed){
        uint16_t ledCount = 0;
        if (myLamp.isMapping()){
            for (uint8_t y = _h-1 - lastLed / mx.cfg.w(); y >= _h - (ledsQt+lastLed) / mx.cfg.w(); y--){
                for (uint8_t x = 0; x < mx.cfg.w(); x++){
                    if (ledCount >= ledsQt){
                        LOG(printf_P, PSTR("Stream: E1.31 out of leds count \n"));
                        return;
                    }
                    if (myLamp.isDirect())
                        EffectMath::getPixel(x, y) = ledsPtr[ledCount];
                    else {
#ifdef EXT_STREAM_BUFF
                        myLamp.writeStreamBuff(ledsPtr[ledCount], x, y);
#else
                        myLamp.writeDrawBuf(ledsPtr[ledCount], x, y);
#endif
                    }
                    ledCount++;
                }
                if (y == 0) break;
            }
        }
        else {
            if (myLamp.isDirect())
                memcpy(getUnsafeLedsArray(), ledsPtr, ledsQt * 3);
            else {
                for (uint16_t i = 0; i < ledsQt; i++)
#ifdef EXT_STREAM_BUFF
                    myLamp.writeStreamBuff(ledsPtr[i], i);
#else
                    myLamp.writeDrawBuf(ledsPtr[i], i);
#endif
            }
        }
    }

void Led_Stream::handleWSPacket(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len){
    if(!ledStream || ledStream->getStreamType() != SOUL_MATE) return;

    if(type == WS_EVT_CONNECT){

        if ((*(AsyncWebServerRequest*)arg).hasArg("config")));
            ledStream->id = client->id();
        LOG(printf_P, PSTR("Stream: ws[%s][%u] connect MEM: %u\n"), server->url(), client->id(), ESP.getFreeHeap());

    } else
    if(type == WS_EVT_DISCONNECT){
        LOG(printf_P, PSTR("Stream: ws[%s][%u] disconnect\n"), server->url(), client->id());
    } else
    if(type == WS_EVT_ERROR){
        LOG(printf_P, PSTR("Stream: ws[%s][%u] error(%u): %s\n"), server->url(), client->id(), *((uint16_t*)arg), (char*)data);
    } else
    if(type == WS_EVT_PONG){
        LOG(printf_P, PSTR("Stream: ws[%s][%u] pong[%u]: %s\n"), server->url(), client->id(), len, (len)?(char*)data:"");
    } else
    if(type == WS_EVT_DATA){

    AwsFrameInfo *info = (AwsFrameInfo*)arg;
    bool pgkReady = false;
    if (info->len == len)
        pgkReady = true;
        static uint32_t lastLen = 0;
        if (info->len > _w*_h*3) {
            LOG(printf_P, PSTR("Stream: ws package out of max leds size %d byte \n"), _w*_h*3);
            return;
        }
        if (info->len != len){
            if (info->index == 0) {
                ledStream->clearWSbuff();
                ledStream->wsBuff = new uint8_t[info->len];
                ledStream->buffSize = info->len;
                if (!ledStream->wsBuff) {
                    LOG(printf_P, PSTR("Stream: ws package out of mem size \n"));
                    return;
                }
                lastLen = 0;
            }
            if (lastLen != info->index || !ledStream->wsBuff) {
                LOG(printf_P, PSTR("Stream: : ws package lost \n"));
                    ledStream->clearWSbuff();
                return;
            }
            memcpy(ledStream->wsBuff+info->index, data, len);
            pgkReady = info->index + len == info->len && ledStream->wsBuff ? true : false;
            lastLen += len;
        }
        if(info->final && pgkReady){
            ledStream->fillBuff(ledStream->wsBuff);
        }
    }
}


Led_Stream::Led_Stream(const STREAM_TYPE type, uint16_t width, uint16_t height)  : _w(width), _h(height) {
    LOG(printf_P, PSTR("Stream ON, type %d \n"), (uint8_t)type);
    streamType = type;
    if (type == E131) {
        firstUni = embui.param(TCONST_Universe).toInt();
        e131 = new ESPAsyncE131(&handleE131Packet);
        // bufLeds = new CRGB[NUM_LEDS]{CRGB::Black};
        lastSeqNum = new uint8_t[uniQt];
        e131->begin(multicast, e131Port, firstUni, uniQt);
    }
    else if (type == SOUL_MATE){
        wsStream.onEvent(handleWSPacket);
        embui.server.addHandler(&wsStream);
        wsStream.enable(true);
    }
    if (myLamp.isDirect()){
        myLamp.effectsTimer(T_DISABLE);
        FastLED.clear();
        FastLED.show();
    }
#ifdef EXT_STREAM_BUFFER
    else
        myLamp.setStreamBuff(true);
#else
    else {
        if(myLamp.isDrawOn()) myLamp.setDraw(false);   // TODO: переделать
        myLamp.setDrawBuff(true);
    }
#endif
}

Led_Stream::~Led_Stream(){
    LOG(printf_P, PSTR("Stream OFF, Type %d \n"), streamType);
    Led_Stream::clearBuff();
    if (streamType == E131) {
        delete[] lastSeqNum;
        delete e131;
        // delete[] bufLeds;
        lastSeqNum = nullptr;
        e131 = nullptr;
        bufLeds = nullptr;
    }
    else if (streamType == SOUL_MATE){
    //     embui.server.removeHandler(&wsStream);
        wsStream.enable(false);
    //     if (wsStream){
    //     delete wsStream;
    //     wsStream = nullptr;
    //     }
    }
    if (myLamp.isDirect())
        myLamp.effectsTimer(T_ENABLE);
#ifdef EXT_STREAM_BUFFER
    else 
        myLamp.setStreamBuff(false);
#endif
}

void Led_Stream::handle() {
    if (isConnected && millis() - lastFrameTimer > TASK_SECOND) {
        clearBuff(); isConnected = false; 
        if (myLamp.isDirect()) 
            myLamp.effectsTimer(T_ENABLE);
    }
    if (id) {
        sendConfig(id);
        id = 0;
    }
}

void Led_Stream::clearWSbuff(){
    if (wsBuff) {
        delete[] wsBuff;
        wsBuff = nullptr;
    }
}

void Led_Stream::fillBuff() {
    if (myLamp.isDirect()){
        if (!isConnected)
            myLamp.effectsTimer(T_DISABLE);
        if (myLamp.isMapping()){
            for(uint8_t x = 0; x < mx.cfg.w(); x++) {
                for (uint8_t y = 0; y < _h; y++) {
                    EffectMath::getPixel(x, y) = bufLeds[getPixelNumE131(x, y)];
                }
            }
        }
        else {
            memcpy(getUnsafeLedsArray(), bufLeds, _w*_h);
        }
        FastLED.show();
    }
    else {
        if (myLamp.isMapping()){
            for(uint8_t x = 0; x < _w; x++) {
                for (uint8_t y = 0; y < _h; y++) {
#ifdef EXT_STREAM_BUFFER
                    myLamp.writeStreamBuff(bufLeds[getPixelNumE131(x, y)], x, y);
#else
                    myLamp.writeDrawBuf(bufLeds[getPixelNumE131(x, y)], x, y);
#endif
                }
            }
        }
        else { 
            for (uint16_t i = 0; i < _w*_h; i++)
#ifdef EXT_STREAM_BUFFER
                myLamp.writeStreamBuff(bufLeds[i], i);
#else
                myLamp.writeDrawBuf(bufLeds[i], i);
#endif
        }
    }
    lastFrameTimer = millis();
    isConnected = true;
}

void Led_Stream::fillBuff(const uint8_t *col){
    if (myLamp.isDirect()){
        if (!isConnected)
            myLamp.effectsTimer(T_DISABLE);
        if (myLamp.isMapping()) {
            for(uint8_t x = 0; x < _w; x++) {
                for (uint8_t y = 0; y < _h; y++) {
                    uint16_t i = mx.transpose(_w-1-x, y) * 3;
                    mx.pixel(x, y) = (CRGB&)col[i];
                    //uint16_t i = EffectMath::getPixelNumberBuff((_w-1) - x, y, _w, _h) * 3;
                    //EffectMath::getPixel(x, y) = (CRGB&)col[i];
                }
            }
        }
        else {
            for (size_t i = 0; i < (buffSize < _w*_h*3 ? buffSize : _w*_h*3); i+=3) {
                leds[i/3] = (CRGB&)col[i];
            }
        }
        FastLED.show();
    }
    else {
        if (myLamp.isMapping()) {
            for(uint8_t x = 0; x < _w; x++) {
                for (uint8_t y = 0; y < _h; y++) {
                    uint16_t i = EffectMath::getPixelNumberBuff((_w-1) - x, y, _w, _h) * 3;
#ifdef EXT_STREAM_BUFFER
                    myLamp.writeStreamBuff((CRGB&)col[i], x, y);
#else
                    myLamp.writeDrawBuf((CRGB&)col[i], x, y);
#endif
                }
            }
        }
        else {
            for (size_t i = 0; i < (buffSize < _w*_h*3 ? buffSize : _w*_h*3); i+=3) {
#ifdef EXT_STREAM_BUFFER
                myLamp.writeStreamBuff((CRGB&)col[i], i/3);
#else
                myLamp.writeDrawBuf((CRGB&)col[i], i/3);
#endif
            }
        }
    }
    lastFrameTimer = millis();
    isConnected = true;
}

void Led_Stream::sendConfig(uint32_t id){       // TODO: доработать
    StaticJsonDocument<EMBUI_IFACE_STA_JSON_SIZE> obj;
    obj["name")] = String("FireLamp-")) + String(embui.mc);
    // obj["version")] = PGversion;
    obj["version")] = "2.5.0");
    obj["cols")] = String(_h);
    obj["rows")] = String(_w);
    obj["ledType")] = "ws2812b");
    obj["serpentine")] = String(MATRIX_TYPE);
    String message;
    serializeJson(obj, message);
    wsStream.text(id, message);
}

void Led_Stream::clearStreamObj(){
    if (ledStream) {
        delete ledStream;
        ledStream = nullptr;
    }
}

void Led_Stream::clearBuff(){
    if (ledStream){
        if (myLamp.isDirect()){
            FastLED.clear();
            FastLED.show();
        }
        else {
#ifdef EXT_STREAM_BUFFER
            myLamp.clearStreamBuff();
#else
            myLamp.clearDrawBuf();
#endif
        }
    }
}

void Led_Stream::newStreamObj(STREAM_TYPE type){
    clearStreamObj();
    ledStream = new Led_Stream(type);
}

#endif