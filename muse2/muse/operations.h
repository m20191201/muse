//=========================================================
//  MusE
//  Linux Music Editor
//    operations.h 
//  (C) Copyright 2014 Tim E. Real (terminator356 on users dot sourceforge dot net)
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; version 2 of
//  the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//
//=========================================================

#ifndef __OPERATIONS_H__
#define __OPERATIONS_H__

#include <list> 
#include <map> 

#include "event.h"
#include "midictrl.h" 
#include "tempo.h" 
#include "al/sig.h" 
#include "keyevent.h"
#include "part.h"
#include "track.h"
#include "route.h"
#include "mididev.h"
#include "midiport.h"
#include "instruments/minstrument.h"

namespace MusECore {


// New items created in GUI thread awaiting addition in audio thread.
struct PendingOperationItem
{
  enum PendingOperationType { ModifySongLength,
                              AddMidiInstrument, DeleteMidiInstrument,
                              AddMidiDevice,     DeleteMidiDevice,
                              AddTrack ,         DeleteTrack, MoveTrack,                    ModifyTrackName,
                              AddPart,           DeletePart,  MovePart,  ModifyPartLength,  ModifyPartName,
                              AddEvent,          DeleteEvent,
                              AddMidiCtrlVal,    DeleteMidiCtrlVal,      ModifyMidiCtrlVal, AddMidiCtrlValList, 
                              AddTempo,          DeleteTempo,            ModifyTempo,       SetGlobalTempo, 
                              AddSig,            DeleteSig,              ModifySig,
                              AddKey,            DeleteKey,              ModifyKey,
                              AddAuxSendValue,   
                              SetMidiPortDevice
                              }; 
                              
  PendingOperationType _type;

  union {
          Part* _part;
          MidiPort* _midi_port;
          void* _void_track_list;
  };
  
  union {
          MidiCtrlValListList* _mcvll;
          TempoList* _tempo_list;  
          AL::SigList* _sig_list; 
          KeyList* _key_list;
          PartList* _part_list; 
          TrackList* _track_list;
          MidiDeviceList* _midi_device_list;
          MidiInstrumentList* _midi_instrument_list;
          AuxSendValueList* _aux_send_value_list;
  };
            
  union {
    MidiInstrument* _midi_instrument;
    MidiDevice* _midi_device;
    Track* _track;
    MidiCtrlValList* _mcvl;
    TEvent* _tempo_event; 
    AL::SigEvent* _sig_event; 
  };

  iPart _iPart; 
  Event _ev;
  iEvent _iev;
  iMidiCtrlVal _imcv;
  iTEvent _iTEvent;
  AL::iSigEvent _iSigEvent;
  iKeyEvent _iKeyEvent;
  iMidiInstrument _iMidiInstrument;
  iMidiDevice _iMidiDevice;
  
  union {
    int _intA;
    const char* _name; 
    double _aux_send_value;
    int _insert_at;
    int _from_idx;
  };
  
  union {
    int _intB;
    int _to_idx;
  };
  
  union {
    int _intC;
  };
  
  union {
    int _intD;
  };
  
  
  PendingOperationItem(MidiPort* midi_port, MidiDevice* midi_device, PendingOperationType type = SetMidiPortDevice)
    { _type = type; _midi_port = midi_port; _midi_device = midi_device; }
    
  PendingOperationItem(AuxSendValueList* asvl, double val, PendingOperationType type = AddAuxSendValue)
    { _type = type; _aux_send_value_list = asvl; _aux_send_value = val; }
    
  PendingOperationItem(MidiInstrumentList* mil, MidiInstrument* midi_instrument, PendingOperationType type = AddMidiInstrument)
    { _type = type; _midi_instrument_list = mil; _midi_instrument = midi_instrument; }
    
  PendingOperationItem(MidiInstrumentList* mil, const iMidiInstrument& imi, PendingOperationType type = DeleteMidiInstrument)
    { _type = type; _midi_instrument_list = mil; _iMidiInstrument = imi; }

  PendingOperationItem(MidiDeviceList* mdl, MidiDevice* midi_device, PendingOperationType type = AddMidiDevice)
    { _type = type; _midi_device_list = mdl; _midi_device = midi_device; }
    
  PendingOperationItem(MidiDeviceList* mdl, const iMidiDevice& imd, PendingOperationType type = DeleteMidiDevice)
    { _type = type; _midi_device_list = mdl; _iMidiDevice = imd; }

  PendingOperationItem(TrackList* tl, Track* track, int insert_at, PendingOperationType type = AddTrack, void* sec_track_list = 0)
    { _type = type; _track_list = tl; _track = track; _insert_at = insert_at; _void_track_list = sec_track_list; }
    
  PendingOperationItem(TrackList* tl, Track* track, PendingOperationType type = DeleteTrack, void* sec_track_list = 0)
    { _type = type; _track_list = tl; _track = track; _void_track_list = sec_track_list; }
    
  PendingOperationItem(TrackList* tl, int from_idx, int to_idx, PendingOperationType type = MoveTrack)
    { _type = type; _track_list = tl; _from_idx = from_idx; _to_idx = to_idx; }

  PendingOperationItem(Track* track, const char* new_name, PendingOperationType type = ModifyTrackName)
    { _type = type; _track = track; _name = new_name; }
    
    
  PendingOperationItem(Part* part, const char* new_name, PendingOperationType type = ModifyPartName)
    { _type = type; _part = part; _name = new_name; }
    
  // new_len must already be in the part's time domain (ticks or frames).
  PendingOperationItem(Part* part, int new_len, PendingOperationType type = ModifyPartLength)
    { _type = type; _part = part; _intA = new_len; }
  
  // Erases ip from part->track()->parts(), then adds part to new_track. NOTE: ip may be part->track()->parts()->end().
  // new_pos must already be in the part's time domain (ticks or frames).
  PendingOperationItem(iPart ip, Part* part, int new_pos, PendingOperationType type = MovePart, Track* new_track = 0)
    { _type = type; _iPart = ip; _part = part; _track = new_track; _intA = new_pos;}
    
  PendingOperationItem(PartList* pl, Part* part, PendingOperationType type = AddPart)
    { _type = type; _part_list = pl; _part = part; }
  
  PendingOperationItem(PartList* pl, const iPart& ip, PendingOperationType type = DeletePart)
    { _type = type; _part_list = pl; _iPart = ip; }
    
    
  PendingOperationItem(Part* part, const Event& ev, PendingOperationType type = AddEvent)
    { _type = type; _part = part; _ev = ev; }
    
  // NOTE: To avoid possibly deleting the event in RT stage 2 when the event is erased from the list, 
  //        _ev is used simply to hold a reference until non-RT stage 3 or after, when the list is cleared.
  PendingOperationItem(Part* part, const iEvent& iev, PendingOperationType type = DeleteEvent)
    { _type = type; _part = part; _iev = iev; _ev = iev->second; }


    PendingOperationItem(MidiCtrlValListList* mcvll, MidiCtrlValList* mcvl, int channel, int control_num, PendingOperationType type = AddMidiCtrlValList)
    { _type = type; _mcvll = mcvll; _mcvl = mcvl; _intA = channel; _intB = control_num; }
    
  PendingOperationItem(MidiCtrlValList* mcvl, Part* part, int tick, int val, PendingOperationType type = AddMidiCtrlVal)
    { _type = type; _mcvl = mcvl; _part = part; _intA = tick; _intB = val; }
    
  PendingOperationItem(MidiCtrlValList* mcvl, const iMidiCtrlVal& imcv, PendingOperationType type = DeleteMidiCtrlVal)
    { _type = type; _mcvl = mcvl; _imcv = imcv; }
    
  // NOTE: mcvl is supplied in case the operation needs to be merged, or transformed into an AddMidiCtrlVal.
  PendingOperationItem(MidiCtrlValList* mcvl, const iMidiCtrlVal& imcv, int val, PendingOperationType type = ModifyMidiCtrlVal)
    { _type = type; _mcvl = mcvl; _imcv = imcv; _intA = val; }
  
  
  // NOTE: 'tick' is the desired tick. te is a new TEvent with tempo and (same) desired tick. Swapping with NEXT event is done.
  PendingOperationItem(TempoList* tl, TEvent* te, int tick, PendingOperationType type = AddTempo)
    { _type = type; _tempo_list = tl; _tempo_event = te; _intA = tick; }
    
  // NOTE: _tempo_event is required. We must erase 'ite' in stage 2, then delete the TEvent* in stage 3 (not stage 1),
  //        so 'ite' is unavailable to fetch the TEvent* from it (in ite->second).
  PendingOperationItem(TempoList* tl, const iTEvent& ite, PendingOperationType type = DeleteTempo)
    { _type = type; _tempo_list = tl; _iTEvent = ite; _tempo_event = ite->second; }
    
  PendingOperationItem(TempoList* tl, const iTEvent& ite, int tempo, PendingOperationType type = ModifyTempo)
    { _type = type; _tempo_list = tl; _iTEvent = ite; _intA = tempo; }
    
  PendingOperationItem(TempoList* tl, int tempo, PendingOperationType type = SetGlobalTempo)
    { _type = type; _tempo_list = tl; _intA = tempo; }

    
  // NOTE: 'tick' is the desired tick. se is a new SigEvent with sig and (same) desired tick. Swapping with NEXT event is done.
  PendingOperationItem(AL::SigList* sl, AL::SigEvent* se, int tick, PendingOperationType type = AddSig)
    { _type = type; _sig_list = sl; _sig_event = se; _intA = tick; }
    
  // NOTE: _sig_event is required. We must erase 'ise' in stage 2, then delete the SigEvent* in stage 3 (not stage 1),
  //        so 'ise' is unavailable to fetch the SigEvent* from it (in ise->second).
  PendingOperationItem(AL::SigList* sl, const AL::iSigEvent& ise, PendingOperationType type = DeleteSig)
    { _type = type; _sig_list = sl; _iSigEvent = ise; _sig_event = ise->second; }
    
  PendingOperationItem(AL::SigList* sl, const AL::iSigEvent& ise, const AL::TimeSignature& s, PendingOperationType type = ModifySig)
    { _type = type; _sig_list = sl; _iSigEvent = ise; _intA = s.z; _intB = s.n; }
    
    
  // NOTE: 'tick' is the desired tick. ke is a new SigEvent with sig and (same) desired tick. Swapping with NEXT event is done.
  PendingOperationItem(KeyList* kl, key_enum ke, int tick, PendingOperationType type = AddKey)
    { _type = type; _key_list = kl; _intA = tick; _intB = ke; }
    
  PendingOperationItem(KeyList* kl, const iKeyEvent& ike, PendingOperationType type = DeleteKey)
    { _type = type; _key_list = kl; _iKeyEvent = ike; }
    
  PendingOperationItem(KeyList* kl, const iKeyEvent& ike, key_enum ke, PendingOperationType type = ModifyKey)
    { _type = type; _key_list = kl; _iKeyEvent = ike; _intA = ke; }
    
  PendingOperationItem(int len, PendingOperationType type = ModifySongLength)
    { _type = type; _intA = len; }
    
  // Execute the operation. Called only from RT stage 2.
  void executeRTStage(); 
  // Execute the operation. Called only from post RT stage 3.
  void executeNonRTStage(); 
  // Get an appropriate indexing value from ops like AddEvent that use it. Other ops like AddMidiCtrlValList return their type (rather than say, zero).
  int getIndex() const;
  // Whether the two special allocating ops (like AddMidiCtrlValList) are the same. 
  // The comparison ignores the actual allocated value, so that such commands can be found before they do their allocating.
  bool isAllocationOp(const PendingOperationItem&) const;
};

class PendingOperationList : public std::list<PendingOperationItem> 
{
  private:
    // Holds sorted version of list. Index is time value for items which have it like events or parts,
    //  otherwise it is the operation type for other items. It doesn't matter too much that ticks and frames
    //  are mixed here, sorting by time is just to speed up searches, we look for operation types.
    std::multimap<int, iterator, std::less<int> > _map; 
  
  public: 
    // Add an operation. Returns false if already exists, otherwise true. Optimizes all added items (merge, discard, alter, embellish etc.)
    bool add(PendingOperationItem);       
    // Execute the RT portion of the operations contained in the list. Called only from RT stage 2.
    void executeRTStage();                 
    // Execute the Non-RT portion of the operations contained in the list. Called only from post RT stage 3.
    void executeNonRTStage();              
    // Clear both the list and the map. 
    void clear();                       
    
    // Find an existing special allocation command (like AddMidiCtrlValList). 
    // The comparison ignores the actual allocated value, so that such commands can be found before they do their allocating.
    iterator findAllocationOp(const PendingOperationItem& op);
};

typedef PendingOperationList::iterator iPendingOperation;
typedef std::multimap<int, iPendingOperation, std::less<int> >::iterator iPendingOperationSorted;
typedef std::multimap<int, iPendingOperation, std::less<int> >::reverse_iterator riPendingOperationSorted;
typedef std::pair <iPendingOperationSorted, iPendingOperationSorted> iPendingOperationSortedRange;

} // namespace MusECore

#endif