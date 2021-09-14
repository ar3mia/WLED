#include "wled.h"

/*
 * WebSockets server for bidirectional communication
 */
#ifdef WLED_ENABLE_WEBSOCKETS

uint16_t wsLiveClientId = 0;
unsigned long wsLastLiveTime = 0;
//uint8_t* wsFrameBuffer = nullptr;

#define WS_LIVE_INTERVAL 40

void wsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len)
{
  if(type == WS_EVT_CONNECT){
    //client connected
    sendDataWs(client);
  } else if(type == WS_EVT_DISCONNECT){
    //client disconnected
    if (client->id() == wsLiveClientId) wsLiveClientId = 0;
  } else if(type == WS_EVT_DATA){
    //data packet
    AwsFrameInfo * info = (AwsFrameInfo*)arg;
    if(info->final && info->index == 0 && info->len == len){
      //the whole message is in a single frame and we got all of its data (max. 1450byte)
      if(info->opcode == WS_TEXT)
      {
        if (len > 0 && len < 10 && data[0] == 'p') {
          //application layer ping/pong heartbeat.
          //client-side socket layer ping packets are unresponded (investigate)
          client->text(F("pong"));
          return;
        }
        bool verboseResponse = false;
        { //scope JsonDocument so it releases its buffer
          DynamicJsonDocument jsonBuffer(JSON_BUFFER_SIZE);
          DeserializationError error = deserializeJson(jsonBuffer, data, len);
          JsonObject root = jsonBuffer.as<JsonObject>();
          if (error || root.isNull()) return;

          if (root["v"] && root.size() == 1) {
            //if the received value is just "{"v":true}", send only to this client
            verboseResponse = true;
          } else if (root.containsKey("lv"))
          {
            wsLiveClientId = root["lv"] ? client->id() : 0;
          } else {
            fileDoc = &jsonBuffer;
            verboseResponse = deserializeState(root);
            fileDoc = nullptr;
            if (!interfaceUpdateCallMode) {
              //special case, only on playlist load, avoid sending twice in rapid succession
              if (millis() - lastInterfaceUpdate > 1700) verboseResponse = false;
            }
          }
        }
        //update if it takes longer than 300ms until next "broadcast"
        if (verboseResponse && (millis() - lastInterfaceUpdate < 1700 || !interfaceUpdateCallMode)) sendDataWs(client);
      }
    } else {
      //message is comprised of multiple frames or the frame is split into multiple packets
      //if(info->index == 0){
        //if (!wsFrameBuffer && len < 4096) wsFrameBuffer = new uint8_t[4096];
      //}

      //if (wsFrameBuffer && len < 4096 && info->index + info->)
      //{

      //}

      if((info->index + len) == info->len){
        if(info->final){
          if(info->message_opcode == WS_TEXT) {
            client->text(F("{\"error\":9}")); //we do not handle split packets right now
          }
        }
      }
    }
  } else if(type == WS_EVT_ERROR){
    //error was received from the other end

  } else if(type == WS_EVT_PONG){
    //pong message was received (in response to a ping request maybe)

  }
}

void sendDataWs(AsyncWebSocketClient * client)
{
  if (!ws.count()) return;
  AsyncWebSocketMessageBuffer * buffer;

  { //scope JsonDocument so it releases its buffer
    DynamicJsonDocument doc(JSON_BUFFER_SIZE);
    JsonObject state = doc.createNestedObject("state");
    serializeState(state);
    JsonObject info  = doc.createNestedObject("info");
    serializeInfo(info);
    size_t len = measureJson(doc);
    buffer = ws.makeBuffer(len);
    if (!buffer) return; //out of memory

    serializeJson(doc, (char *)buffer->get(), len +1);
  } 
  if (client) {
    client->text(buffer);
  } else {
    ws.textAll(buffer);
  }
}

void handleWs()
{
  if (millis() - wsLastLiveTime > WS_LIVE_INTERVAL)
  {
    ws.cleanupClients();
    bool success = true;
    if (wsLiveClientId)
      success = serveLiveLeds(nullptr, wsLiveClientId);
    wsLastLiveTime = millis();
    if (!success) wsLastLiveTime -= 20; //try again in 20ms if failed due to non-empty WS queue
  }
}

#else
void handleWs() {}
void sendDataWs(AsyncWebSocketClient * client) {}
#endif