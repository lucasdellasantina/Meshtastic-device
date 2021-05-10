#pragma once
#include "ProtobufPlugin.h"

/**
 * MensajePlugin plugin for sending/receiving NodeInfos into the mesh
 */
class MensajePlugin : public ProtobufPlugin<Data>, private concurrency::OSThread
{
    /// The id of the last packet we sent, to allow us to cancel it if we make something fresher
    PacketId prevPacketId = 0;
    
    uint32_t currentGeneration = 0;
  public:
    /** Constructor
     * name is for debugging output
     */
    MensajePlugin();
    
    /**
     * Send our NodeInfo into the mesh
     */
    void sendMensaje(NodeNum dest = NODENUM_BROADCAST, bool wantReplies = false);

    /** Does our periodic broadcast */
    virtual int32_t runOnce();
  protected:
    /** Called to handle a particular incoming message

    @return true if you've guaranteed you've handled this message and no other handlers should be considered for it
    */
    virtual bool handleReceivedProtobuf(const MeshPacket &mp, const Data *p);

    /** Messages can be received that have the want_response bit set.  If set, this callback will be invoked
     * so that subclasses can (optionally) send a response back to the original sender.  */
    virtual MeshPacket *allocReply();
};

extern MensajePlugin *mensajePlugin;