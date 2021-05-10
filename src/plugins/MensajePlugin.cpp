#include "MensajePlugin.h"
#include "MeshService.h"
//#include "NodeDB.h"
#include "RTC.h"
#include "Router.h"
#include "configuration.h"
#include "main.h"
#include <stdlib.h>


MensajePlugin *mensajePlugin;

bool MensajePlugin::handleReceivedProtobuf(const MeshPacket &mp, const Data *pptr) //esta maneja que pasa cuando se recive (teoriacamente se tienen q procesar aca)
{
    auto p = *pptr;
    bool wasBroadcast = mp.to == NODENUM_BROADCAST;

    // Show new nodes on LCD screen
    if (wasBroadcast) {
        String lcd = String("Recibi: ");
        for(int ii=0; ii<p.payload.size; ii++)  //esta bien echo segun nachooo
            lcd += p.payload.bytes[ii];
        lcd +=  + "\n";
        if(screen)
            screen->print(lcd.c_str());
    }

    // DEBUG_MSG("did handleReceived\n");
    return false; // Let others look at this message also if they want
}

void MensajePlugin::sendMensaje(NodeNum dest, bool wantReplies) //aca se envian y tambian se modifica como e desde el tipo  &datos  que se puede cambiar pero hay q buscarlo en mesh.proto en mesh protobuf q es un sub module de git

{
    // cancel any not yet sent (now stale) position packets
    if (prevPacketId) // if we wrap around to zero, we'll simply fail to cancel in that rare case (no big deal)
        service.cancelSending(prevPacketId);

    MeshPacket *p = allocReply();
    p->to = dest;
    p->id = (uint32_t)rand();
    p->decoded.want_response = wantReplies;
    p->priority = MeshPacket_Priority_BACKGROUND;
    prevPacketId = p->id;

    service.sendToMesh(p);
}

MeshPacket *MensajePlugin::allocReply()
{
    const Data &datos = {PortNum_UNKNOWN_APP, // No se usa
                            {   3, // Aca van la cantidad de bytes que se estan mandando abajo 
                                {4,5,6} // Este es el payload realmente
                            },
                        0, 0, 0, 0 // Datos que no se usan};

    DEBUG_MSG("MENSAJE ENVIADO: %d,%d,%d,%d,%d,%d,%d\n",
        datos.portnum, datos.payload.size, datos.payload.bytes[0], datos.want_response, datos.dest, datos.source, datos.request_id);
    return allocDataProtobuf(datos);
}

MensajePlugin::MensajePlugin()
    : ProtobufPlugin("mensaje", PortNum_MENSAJE_APP, Data_fields), concurrency::OSThread("MensajePlugin")
{
    isPromiscuous = true; // We always want to update our nodedb, even if we are sniffing on others
    setIntervalFromNow(30 *
                       1000); // Send our initial owner announcement 30 seconds after we start (to give network time to setup)
}

int32_t MensajePlugin::runOnce()
{
    //static uint32_t currentGeneration;

    // If we changed channels, ask everyone else for their latest info
    //bool requestReplies = currentGeneration != radioGeneration;
    //currentGeneration = radioGeneration;
    bool requestReplies = true;

    DEBUG_MSG("ESTOY POR ENVIAR EL MENSAJE, quiero respuestas?: %d\n", requestReplies);
    sendMensaje(NODENUM_BROADCAST, requestReplies); // Send our info (don't request replies)

    return getPref_position_broadcast_secs() * 1000;
}

