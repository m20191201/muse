//=========================================================
//  MusE
//  Linux Music Editor
//    operations.cpp 
//  (C) Copyright 2014, 2016 Tim E. Real (terminator356 on users dot sourceforge dot net)
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

#include "operations.h"
#include "song.h"

// Enable for debugging:
//#define _PENDING_OPS_DEBUG_

// For debugging output: Uncomment the fprintf section.
#define ERROR_OPERATIONS(dev, format, args...)  fprintf(dev, format, ##args)
#define DEBUG_OPERATIONS(dev, format, args...)  //fprintf(dev, format, ##args)

namespace MusECore {

//-----------------------------------
//  MidiCtrlValListIterators
//-----------------------------------
  
MidiCtrlValListIterators::iterator MidiCtrlValListIterators::findList(const MidiCtrlValList* valList)
{
  for(iterator i = begin(); i != end(); ++i)
    if((*i)->second == valList)
      return i;
  return end();
}
  
MidiCtrlValListIterators::const_iterator MidiCtrlValListIterators::findList(const MidiCtrlValList* valList) const
{
  for(const_iterator i = begin(); i != end(); ++i)
    if((*i)->second == valList)
      return i;
  return end();
}
  
//-----------------------------------
//  MidiCtrlValLists2bErased
//-----------------------------------
  
void MidiCtrlValLists2bErased::add(int port, const iMidiCtrlValList& item)
{
  iterator i = find(port);
  if(i == end())
  {
    MidiCtrlValListIterators mcvli;
    mcvli.push_back(item);
    insert(MidiCtrlValLists2bErasedInsertPair_t(port, mcvli));
    return;
  }
  MidiCtrlValListIterators& mcvli = i->second;
  for(iMidiCtrlValListIterators_t imcvli = mcvli.begin(); imcvli != mcvli.end(); ++imcvli)
  {
    iMidiCtrlValList imcvl = *imcvli;
    // Compare list pointers.
    if(imcvl->second == item->second)
      return; // Already exists.
  }
  mcvli.push_back(item);
}

MidiCtrlValLists2bErased::iterator MidiCtrlValLists2bErased::findList(int port, const MidiCtrlValList* valList)
{
  iterator i = find(port);
  if(i == end())
    return end();
  if(i->second.findList(valList) != i->second.end())
    return i;
  return end();
}

MidiCtrlValLists2bErased::const_iterator MidiCtrlValLists2bErased::findList(int port, const MidiCtrlValList* valList) const
{
  const_iterator i = find(port);
  if(i == end())
    return end();
  if(i->second.findList(valList) != i->second.end())
    return i;
  return end();
}
  
//-----------------------------------
//  PendingOperationItem
//-----------------------------------

bool PendingOperationItem::isAllocationOp(const PendingOperationItem& op) const
{
  switch(op._type)
  {
    case AddMidiCtrlValList:
      // A is channel B is control.
      if(_type == AddMidiCtrlValList && _mcvll == op._mcvll && _intA == op._intA && _intB == op._intB)
        return true;
    break;
    
    case AddTempo:
      // _posLenVal is tick.
      if(_type == AddTempo && _tempo_list == op._tempo_list && _posLenVal == op._posLenVal)
        return true;
    break;
      
    case AddSig:
      // _posLenVal is tick.
      if(_type == AddSig && _sig_list == op._sig_list && _posLenVal == op._posLenVal)
        return true;
    break;
    
    // In the case of type AddMidiDevice, this searches for the name only.
    case AddMidiDevice:
      if(_type == AddMidiDevice && _midi_device_list == op._midi_device_list &&
         _midi_device->name() == op._midi_device->name())
        return true;
    break;
    
    default:
    break;  
  }
  
  return false;
}

unsigned int PendingOperationItem::getIndex() const
{
  switch(_type)
  {
    case Uninitialized:
    case AddAuxSendValue:
    case AddMidiInstrument:
    case DeleteMidiInstrument:
    case ReplaceMidiInstrument:
    case AddMidiDevice:
    case DeleteMidiDevice:
    case ModifyMidiDeviceAddress:
    case ModifyMidiDeviceFlags:
    case ModifyMidiDeviceName:
    case SetInstrument:
    case AddTrack:
    case DeleteTrack:
    case MoveTrack:
    case ModifyTrackName:
    case ModifyTrackDrumMapItem:
    case ReplaceTrackDrumMapPatchList:
    case RemapDrumControllers:
    case UpdateDrumMaps:
    case SetTrackRecord:
    case SetTrackMute:
    case SetTrackSolo:
    case SetTrackRecMonitor:
    case SetTrackOff:
    case ModifyPartName:
    case ModifySongLength:
    case AddMidiCtrlValList:
    case ModifyAudioCtrlValList:
    case SetGlobalTempo:
    case AddRoute:
    case DeleteRoute:
    case AddRouteNode:
    case DeleteRouteNode:
    case ModifyRouteNode:
    case UpdateSoloStates:
    case EnableAllAudioControllers:
    case GlobalSelectAllEvents:
    case SwitchMetronomeSettings:
    case ModifyMetronomeAccentMap:
    case SetExternalSyncFlag:
    case SetUseJackTransport:
    case SetUseMasterTrack:
    case ModifyAudioSamples:
    case SetStaticTempo:
    case ModifyLocalAudioConverterSettings:
    case ModifyLocalAudioConverter:
    case ModifyDefaultAudioConverterSettings:
    case ModifyStretchListRatio:
    case SetAudioConverterOfflineMode:
      // To help speed up searches of these ops, let's (arbitrarily) set index = type instead of all of them being at index 0!
      return _type;
    
    case ModifyPartLength:
      return _part->posValue();
    
    case MovePart:
      // _part is used here rather than _iPart since _iPart can be end().
      return _part->posValue();
    
    case AddPart:
      return _part->posValue();  
    
    case DeletePart:
      return _iPart->second->posValue();

    case SelectPart:
      return _part->posValue();

      
    case AddEvent:
      return _ev.posValue();
    
    case DeleteEvent:
      return _ev.posValue();
    
    case SelectEvent:
      return _ev.posValue();
      
      
    case AddMidiCtrlVal:
      return _posLenVal;  // Tick
    
    case DeleteMidiCtrlVal:
      return _imcv->first;  // Tick
    
    case ModifyMidiCtrlVal:
      return _imcv->first;  // Tick

    
    case AddAudioCtrlVal:
      return _posLenVal;  // Frame
    
    case DeleteAudioCtrlVal:
      return _iCtrl->first;  // Frame
    
    case ModifyAudioCtrlVal:
      return _iCtrl->first;  // Frame

    
    case AddTempo:
      return _posLenVal;  // Tick
    
    case DeleteTempo:
      return _iTEvent->first;  // Tick
    
    case ModifyTempo:
      // We want the 'real' tick, not _iTEvent->first which is the index of the next iterator! 
      return _iTEvent->second->tick;  // Tick
    
    
    case AddSig:
      return _posLenVal;  // Tick
    
    case DeleteSig:
      return _iSigEvent->first;  // Tick
    
    case ModifySig:
      // We want the 'real' tick, not _iSigEvent->first which is the index of the next iterator! 
      return _iSigEvent->second->tick;  // Tick

    
    case AddKey:
      return _posLenVal;  // Tick
    
    case DeleteKey:
      return _iKeyEvent->first;  // Tick
    
    case ModifyKey:
      // We want the 'real' tick, not _iKeyEvent->first which is the index of the next iterator! 
      return _iKeyEvent->second.tick;  // Tick
    
    case AddStretchListRatioAt:
      return _museFrame;  // Frame
    
    case DeleteStretchListRatioAt:
      return _iStretchEvent->first;  // Frame
    
    case ModifyStretchListRatioAt:
      return _iStretchEvent->first;  // Frame

    default:
      ERROR_OPERATIONS(stderr, "PendingOperationItem::getIndex unknown op type: %d\n", _type);
      return 0;
    break;  
  }
}  

SongChangedStruct_t PendingOperationItem::executeRTStage()
{
  SongChangedStruct_t flags = 0;
  switch(_type)
  {
    case ModifyDefaultAudioConverterSettings:
      DEBUG_OPERATIONS(stderr, "PendingOperationItem::executeRTStage ModifyDefaultAudioConverterSettings: "
                                "settings:%p\n", _audio_converter_settings);
      if(_audio_converter_settings)
      {
        // Grab the current settings.
        AudioConverterSettingsGroup* cur_settings = MusEGlobal::defaultAudioConverterSettings;
        // Now change the actual global pointer variable.
        MusEGlobal::defaultAudioConverterSettings = _audio_converter_settings;
        // Transfer the original pointer into the member, so it can be deleted in the non-RT stage.
        _audio_converter_settings = cur_settings;
        flags |= SC_AUDIO_CONVERTER;
      }
    break;
    
    case ModifyLocalAudioConverterSettings:
    {
      DEBUG_OPERATIONS(stderr, "PendingOperationItem::executeRTStage ModifyLocalAudioConverterSettings: "
                                "sndFile:%p settings:%p\n", 
                                *_sndFileR, _audio_converter_settings);

      // _audio_converter_settings can be NULL meaning don't touch, settings can only be 
      //  'replaced' but not deleted.
      if(_audio_converter_settings)
      {
        AudioConverterSettingsGroup* cur_settings = _sndFileR.audioConverterSettings();
        _sndFileR.setAudioConverterSettings(_audio_converter_settings);
        // Transfer the original pointer into the member, so it can be deleted in the non-RT stage.
        _audio_converter_settings = cur_settings;
        flags |= SC_AUDIO_CONVERTER;
      }
    }
    break;
      
    case ModifyLocalAudioConverter:
    {
      DEBUG_OPERATIONS(stderr, "PendingOperationItem::executeRTStage ModifyLocalAudioConverter: "
                                "sndFile:%p audio_converter:%p audio_converter_ui:%p\n", 
                                *_sndFileR, _audio_converter, _audio_converter_ui);

      // _audio_converter and _audio_converter_ui can be NULL meaning delete them.
      AudioConverterPluginI* cur_conv = _sndFileR.staticAudioConverter(AudioConverterSettings::RealtimeMode);
      //if(_audio_converter)
        _sndFileR.setStaticAudioConverter(_audio_converter, AudioConverterSettings::RealtimeMode);
      // Transfer the original pointer into the member, so it can be deleted in the non-RT stage.
      _audio_converter = cur_conv;
      
      AudioConverterPluginI* cur_convUI = _sndFileR.staticAudioConverter(AudioConverterSettings::GuiMode);
      //if(_audio_converter_ui)
        _sndFileR.setStaticAudioConverter(_audio_converter_ui, AudioConverterSettings::GuiMode);
      // Transfer the original pointer into the member, so it can be deleted in the non-RT stage.
      _audio_converter_ui = cur_convUI;
      flags |= SC_AUDIO_CONVERTER;
    }
    break;
      
    case SetAudioConverterOfflineMode:
    {
      DEBUG_OPERATIONS(stderr, "PendingOperationItem::executeRTStage SetAudioConverterOfflineMode: "
                                "sndFile:%p audio_converter:%p\n", 
                                *_sndFile, _audio_converter);

      AudioConverterPluginI* cur_conv = _sndFileR.staticAudioConverter(AudioConverterSettings::RealtimeMode);
      //if(_audio_converter)
        _sndFileR.setStaticAudioConverter(_audio_converter, AudioConverterSettings::RealtimeMode);
      // Transfer the original pointer into the member, so it can be deleted in the non-RT stage.
      _audio_converter = cur_conv;
      flags |= SC_AUDIO_CONVERTER;
    }
    break;


    case ModifyTrackDrumMapItem:
    {
#ifdef _PENDING_OPS_DEBUG_
      fprintf(stderr, "PendingOperationItem::executeRTStage ModifyTrackDrumMapItem drummap operation:%p\n",
              _drum_map_track_operation);
#endif

      MidiTrack* mt;
      MidiTrackList& mtl = _drum_map_track_operation->_tracks;
      for(iMidiTrack imt = mtl.begin(); imt != mtl.end(); ++imt)
      {
        mt = *imt;
        // FIXME Possible non realtime-friendly allocation.
        mt->modifyWorkingDrumMap(_drum_map_track_operation->_workingItemList,
                                 _drum_map_track_operation->_isReset,
                                 _drum_map_track_operation->_includeDefault,
                                 _drum_map_track_operation->_isInstrumentMod,
                                 _drum_map_track_operation->_doWholeMap);
        flags |= (SC_DRUMMAP);
      }

      // If this is an instrument modification we must now do a
      //  general update of all drum track drum maps.
      // Ideally we would like to update only the required ones,
      //  but it is too difficult to tell which maps need updating
      //  from inside the above loop (or inside modifyWorkingDrumMap).
      if(_drum_map_track_operation->_isInstrumentMod)
      {
        MidiTrackList* mtlp = MusEGlobal::song->midis();
        for(iMidiTrack imt = mtlp->begin(); imt != mtlp->end(); ++imt)
        {
          mt = *imt;
          if(mt->type() != Track::NEW_DRUM)
            continue;
          if(mt->updateDrummap(false))
            flags |= (SC_DRUMMAP);
        }
      }
      flags |= (SC_DRUMMAP);
    }
    break;

    case ReplaceTrackDrumMapPatchList:
    {
#ifdef _PENDING_OPS_DEBUG_
      fprintf(stderr, "PendingOperationItem::executeRTStage ReplaceTrackDrumMapPatchList drummap operation:%p\n",
              _drum_map_track_patch_replace_operation);
#endif

      MidiTrack* mt = _drum_map_track_patch_replace_operation->_track;
      WorkingDrumMapPatchList* orig_wdmpl = mt->workingDrumMap();
      // Simply switch pointers. Be sure to delete the original pointers later in the non-realtime stage.
      // After the list pointers have been switched, swap with the replacement so that it can be deleted later.
      mt->setWorkingDrumMap(_drum_map_track_patch_replace_operation->_workingItemPatchList,
                            _drum_map_track_patch_replace_operation->_isInstrumentMod);
      _drum_map_track_patch_replace_operation->_workingItemPatchList = orig_wdmpl;

      // If this is an instrument modification we must now do a
      //  general update of all drum track drum maps.
      // Ideally we would like to update only the required ones,
      //  but it is too difficult to tell which maps need updating
      //  from inside the above loop (or inside modifyWorkingDrumMap).
      if(_drum_map_track_patch_replace_operation->_isInstrumentMod)
      {
        MidiTrackList* mtlp = MusEGlobal::song->midis();
        for(iMidiTrack imt = mtlp->begin(); imt != mtlp->end(); ++imt)
        {
          mt = *imt;
          if(mt->type() != Track::NEW_DRUM)
            continue;
          if(mt->updateDrummap(false))
            flags |= (SC_DRUMMAP);
        }
      }
      flags |= (SC_DRUMMAP);
    }
    break;

    case RemapDrumControllers:
    {
#ifdef _PENDING_OPS_DEBUG_
      fprintf(stderr, "PendingOperationItem::executeRTStage RemapDrumControllers remap operation:%p\n",
              _midi_ctrl_val_remap_operation);
#endif

      for(iMidiCtrlValLists2bErased_t imcvle = _midi_ctrl_val_remap_operation->_midiCtrlValLists2bErased.begin();
          imcvle != _midi_ctrl_val_remap_operation->_midiCtrlValLists2bErased.end(); ++imcvle)
      {
        const int port = imcvle->first;
        MidiCtrlValListIterators& mcvli = imcvle->second;
        MidiPort* mp = &MusEGlobal::midiPorts[port];
        MidiCtrlValListList* mcvll = mp->controller();
        for(iMidiCtrlValListIterators_t imcvli = mcvli.begin(); imcvli != mcvli.end(); ++imcvli)
          mcvll->del(*imcvli);
      }

      for(iMidiCtrlValLists2bAdded_t imcvla = _midi_ctrl_val_remap_operation->_midiCtrlValLists2bAdded.begin();
          imcvla != _midi_ctrl_val_remap_operation->_midiCtrlValLists2bAdded.end(); ++imcvla)
      {
        const int port = imcvla->first;
        MidiCtrlValListList* mcvll_a = imcvla->second;
        MidiPort* mp = &MusEGlobal::midiPorts[port];
        MidiCtrlValListList* mcvll = mp->controller();
        for(iMidiCtrlValList imcvl = mcvll_a->begin(); imcvl != mcvll_a->end(); ++imcvl)
          mcvll->add(imcvl->first >> 24, imcvl->second);
      }

      // TODO: What to use here? We don't have anything SC_* related... yet.
      //flags |= (SC_MIDI_CONTROLLER_ADD);
      flags |= (SC_EVERYTHING);
    }
    break;

    case UpdateDrumMaps:
#ifdef _PENDING_OPS_DEBUG_
      fprintf(stderr, "PendingOperationItem::executeRTStage UpdateDrumMaps: midi_port:%p:\n", _midi_port);
#endif      
      if(_midi_port->updateDrumMaps())
        flags |= SC_DRUMMAP;
    break;
    
    case UpdateSoloStates:
      DEBUG_OPERATIONS(stderr, "PendingOperationItem::executeRTStage UpdateSoloStates: track_list:%p:\n", _track_list);
      // TODO Use the track_list, or simply keep as dummy parameter to identify UpdateSoloStates?
      MusEGlobal::song->updateSoloStates();
      flags |= SC_SOLO;
    break;
    
    // TODO: Try to break this operation down so that only the actual operation is executed stage-2. 
    case AddRoute:
#ifdef _PENDING_OPS_DEBUG_
      DEBUG_OPERATIONS(stderr, "PendingOperationItem::executeRTStage AddRoute: src/dst routes:\n");
      _src_route.dump();
      _dst_route.dump();
#endif      
      if(addRoute(_src_route, _dst_route))
        flags |= SC_ROUTE;
    break;
    
    // TODO: Try to break this operation down so that only the actual operation is executed stage-2. 
    case DeleteRoute:
#ifdef _PENDING_OPS_DEBUG_
      DEBUG_OPERATIONS(stderr, "PendingOperationItem::executeRTStage DeleteRoute: src/dst routes:\n");
      _src_route.dump();
      _dst_route.dump();
#endif      
      if(removeRoute(_src_route, _dst_route))
        flags |= SC_ROUTE;
    break;
    
    case AddRouteNode:
#ifdef _PENDING_OPS_DEBUG_
      DEBUG_OPERATIONS(stderr, "PendingOperationItem::executeRTStage AddRouteNode: route_list:%p route:\n", _route_list);
      _src_route.dump();
#endif      
      _route_list->push_back(_src_route);
      flags |= SC_ROUTE;
    break;
    
    case DeleteRouteNode:
#ifdef _PENDING_OPS_DEBUG_
      DEBUG_OPERATIONS(stderr, "PendingOperationItem::executeRTStage DeleteRouteNode: route_list:%p route:\n", _route_list);
      _iRoute->dump();
#endif      
      _route_list->erase(_iRoute);
      flags |= SC_ROUTE;
    break;
    
    case ModifyRouteNode:
#ifdef _PENDING_OPS_DEBUG_
      DEBUG_OPERATIONS(stderr, "PendingOperationItem::executeRTStage ModifyRouteNode: src/dst routes:\n");
      _src_route.dump();
      _dst_route_pointer->dump();
#endif      
      *_dst_route_pointer = _src_route;
      flags |= SC_ROUTE;
    break;

    case AddAuxSendValue:
      DEBUG_OPERATIONS(stderr, "PendingOperationItem::executeRTStage AddAuxSendValue aux_send_value_list:%p val:%f\n", _aux_send_value_list, _aux_send_value);
      _aux_send_value_list->push_back(_aux_send_value);
    break;

    
    case AddMidiInstrument:
      DEBUG_OPERATIONS(stderr, "PendingOperationItem::executeRTStage AddMidiInstrument instrument_list:%p instrument:%p\n", _midi_instrument_list, _midi_instrument);
      _midi_instrument_list->push_back(_midi_instrument);
      flags |= SC_CONFIG | SC_MIDI_INSTRUMENT | SC_DRUMMAP | SC_MIDI_CONTROLLER_ADD;
    break;
    
    case DeleteMidiInstrument:
      DEBUG_OPERATIONS(stderr, "PendingOperationItem::executeRTStage DeleteMidiInstrument instrument_list:%p instrument:%p\n", _midi_instrument_list, *_iMidiInstrument);
      _midi_instrument_list->erase(_iMidiInstrument);
      flags |= SC_CONFIG | SC_MIDI_INSTRUMENT | SC_DRUMMAP | SC_MIDI_CONTROLLER_ADD;
    break;
    
    case ReplaceMidiInstrument:
    {
#ifdef _PENDING_OPS_DEBUG_
      fprintf(stderr, "PendingOperationItem::executeRTStage ReplaceMidiInstrument instrument_list:%p instrument:%p new_ instrument:%p\n",
              _midi_instrument_list, *_iMidiInstrument, _midi_instrument);
#endif
      // Grab the existing pointer to be deleted.
      MidiInstrument* orig = *_iMidiInstrument;
      // Erase from the list.
      _midi_instrument_list->erase(_iMidiInstrument);
      // Add the new instrument.
      _midi_instrument_list->push_back(_midi_instrument);

      // Change all ports which used the original instrument.
      for(int port = 0; port < MusECore::MIDI_PORTS; ++port)
      {
        MidiPort* mp = &MusEGlobal::midiPorts[port];
        if(mp->instrument() != orig)
          continue;
        // Set the new instrument and nothing more (ie. don't use MidiPort::changeInstrument()).
        // Here we will flag the initializations, and normalize and update the drum maps.
        mp->setInstrument(_midi_instrument);
        // Flag the port to send initializations next time it bothers to check.
        // TODO: Optimize: We only need this if the user changed the initialization
        //        lists or sysex list. Find a way to pass that info here.
        mp->clearInitSent();
      }

      // Since this is an instrument modification we must now do a
      //  general update of all drum track drum maps using this instrument.
      // Ideally we would like to update only the required ones,
      //  but it is too difficult to tell which maps need updating
      //  from inside the above loop (or inside modifyWorkingDrumMap).
      MidiTrack* mt;
      int mt_port;
      MidiPort* mt_mp;
      MidiTrackList* mtlp = MusEGlobal::song->midis();
      for(iMidiTrack imt = mtlp->begin(); imt != mtlp->end(); ++imt)
      {
        mt = *imt;
        if(mt->type() != Track::NEW_DRUM)
          continue;
        mt_port = mt->outPort();
        if(mt_port < 0 || mt_port >= MusECore::MIDI_PORTS)
          continue;
        mt_mp = &MusEGlobal::midiPorts[mt_port];
        // We are looking for tracks which are now using the new instrument.
        if(mt_mp->instrument() != _midi_instrument)
          continue;
        // Ensure there are NO duplicate enote fields.
        //mt->normalizeWorkingDrumMapPatchList();
        // Finally, update the track's drum map (and drum in map).
        mt->updateDrummap(false);
      }

      // Transfer the original pointer back to _midi_instrument so it can be deleted in the non-RT stage.
      _midi_instrument = orig;

      flags |= SC_CONFIG | SC_MIDI_INSTRUMENT | SC_DRUMMAP | SC_MIDI_CONTROLLER_ADD;
    }
    break;

    case AddMidiDevice:
      DEBUG_OPERATIONS(stderr, "PendingOperationItem::executeRTStage AddMidiDevice devicelist:%p device:%p\n", _midi_device_list, _midi_device);
      _midi_device_list->push_back(_midi_device);
      flags |= SC_CONFIG;
    break;
    
    case DeleteMidiDevice:
      DEBUG_OPERATIONS(stderr, "PendingOperationItem::executeRTStage DeleteMidiDevice devicelist:%p device:%p\n", _midi_device_list, *_iMidiDevice);
      _midi_device_list->erase(_iMidiDevice);
      flags |= SC_CONFIG;
    break;
    
    case ModifyMidiDeviceAddress:
      DEBUG_OPERATIONS(stderr, "PendingOperationItem::executeRTStage ModifyMidiDeviceAddress device:%p client:%d port:%d\n", _midi_device, _address_client, _address_port);
      _midi_device->setAddressClient(_address_client);
      _midi_device->setAddressPort(_address_port);
      _midi_device->setOpenFlags(_open_flags);
      flags |= SC_CONFIG;
    break;
    
    case ModifyMidiDeviceFlags:
      DEBUG_OPERATIONS(stderr, "PendingOperationItem::executeRTStage ModifyMidiDeviceFlags device:%p rwFlags:%d openFlags:%d\n", _midi_device, _rw_flags, _open_flags);
      _midi_device->setrwFlags(_rw_flags);
      _midi_device->setOpenFlags(_open_flags);
      flags |= SC_CONFIG;
    break;
    
    case ModifyMidiDeviceName:
      DEBUG_OPERATIONS(stderr, "PendingOperationItem::executeRTStage ModifyMidiDeviceName device:%p name:%s\n", _midi_device, _name->toLocal8Bit().data());
      _midi_device->setName(*_name);
      flags |= SC_CONFIG;
    break;
    
    case SetInstrument:
#ifdef _PENDING_OPS_DEBUG_
      fprintf(stderr, "PendingOperationItem::executeRTStage SetInstrument port:%p instr:%p\n",
              _midi_port, _midi_instrument);
#endif      
      _midi_port->setInstrument(_midi_instrument);
      // Flag the port to send initializations next time it bothers to check.
      _midi_port->clearInitSent();
      flags |= SC_MIDI_INSTRUMENT;
    break;
    
    case AddTrack:
    {
      DEBUG_OPERATIONS(stderr, "PendingOperationItem::executeRTStage AddTrack track_list:%p track:%p\n", _track_list, _track);
      if(_void_track_list)
      {
        switch(_track->type())
        {
              case Track::MIDI:
              case Track::DRUM:
              case Track::NEW_DRUM:
                    static_cast<MidiTrackList*>(_void_track_list)->push_back(static_cast<MidiTrack*>(_track));
                    break;
              case Track::WAVE:
                    static_cast<WaveTrackList*>(_void_track_list)->push_back(static_cast<WaveTrack*>(_track));
                    break;
              case Track::AUDIO_OUTPUT:
                    //--------------------------------------------------------------
                    // Connect metronome audio click
                    //--------------------------------------------------------------
                    // Are there no other existing AudioOutput tracks?
                    if(static_cast<OutputList*>(_void_track_list)->empty())
                      // Do the user a favour: Auto-connect the metronome to the track.
                      static_cast<AudioOutput*>(_track)->setSendMetronome(true);
                    
                    static_cast<OutputList*>(_void_track_list)->push_back(static_cast<AudioOutput*>(_track));
                    break;
              case Track::AUDIO_GROUP:
                    static_cast<GroupList*>(_void_track_list)->push_back(static_cast<AudioGroup*>(_track));
                    break;
              case Track::AUDIO_AUX:
                    static_cast<AuxList*>(_void_track_list)->push_back(static_cast<AudioAux*>(_track));
                    // Special for aux, make it easier to detect their changes.
                    flags |= SC_AUX;
                    break;
              case Track::AUDIO_INPUT:
                    static_cast<InputList*>(_void_track_list)->push_back(static_cast<AudioInput*>(_track));
                    break;
              case Track::AUDIO_SOFTSYNTH:
                    static_cast<SynthIList*>(_void_track_list)->push_back(static_cast<SynthI*>(_track));
                    break;
              default:
                    ERROR_OPERATIONS(stderr, "PendingOperationItem::executeRTStage AddTrack: Unknown track type %d\n", _track->type());
                    return flags;
        }
      }
      
      iTrack track_it = _track_list->index2iterator(_insert_at);
      _track_list->insert(track_it, _track);
      flags |= SC_TRACK_INSERTED;

      // Add routes:
      if(_track->isMidiTrack())
      {
            // Add any port output routes to this track
            const RouteList* rl = _track->inRoutes();
            for(ciRoute r = rl->begin(); r != rl->end(); ++r)
            {
                  switch(r->type)
                  {
                    case Route::MIDI_PORT_ROUTE:  {
                      Route src(_track, r->channel);
                      MusEGlobal::midiPorts[r->midiPort].outRoutes()->push_back(src);
                      flags |= SC_ROUTE; }
                    break;
                    case Route::TRACK_ROUTE:
                    case Route::JACK_ROUTE:
                    case Route::MIDI_DEVICE_ROUTE:
                    break;
                  }
            }
            // Add any port input routes from this track
            rl = _track->outRoutes();
            for(ciRoute r = rl->begin(); r != rl->end(); ++r)
            {
                  switch(r->type)
                  {
                    case Route::MIDI_PORT_ROUTE:  {
                      Route src(_track, r->channel);
                      MusEGlobal::midiPorts[r->midiPort].inRoutes()->push_back(src);
                      flags |= SC_ROUTE; }
                    break;
                    case Route::TRACK_ROUTE:
                    case Route::JACK_ROUTE:
                    case Route::MIDI_DEVICE_ROUTE:
                    break;
                  }
            }      
      }
      else 
      {
            // Add other tracks' output routes to this track
            const RouteList* rl = _track->inRoutes();
            for(ciRoute r = rl->begin(); r != rl->end(); ++r)
            {
                  switch(r->type)
                  {
                    case Route::TRACK_ROUTE: {
                      Route src(_track, r->remoteChannel, r->channels);
                      src.remoteChannel = r->channel;
                      r->track->outRoutes()->push_back(src);
                      flags |= SC_ROUTE;
                      // Is the source an Aux Track or else does it have Aux Tracks routed to it?
                      // Update this track's aux ref count.
                      if(r->track->auxRefCount())
                      {
                        _track->updateAuxRoute(r->track->auxRefCount(), NULL);
                      }
                      else if(r->track->type() == Track::AUDIO_AUX)
                      {
                        _track->updateAuxRoute(1, NULL);
                      }
                    }
                    break;
                    case Route::MIDI_PORT_ROUTE:
                    case Route::JACK_ROUTE:
                    case Route::MIDI_DEVICE_ROUTE:
                    break;
                  }
            }
            // Add other tracks' input routes from this track
            rl = _track->outRoutes();
            for(ciRoute r = rl->begin(); r != rl->end(); ++r)
            {
                  switch(r->type)
                  {
                    case Route::TRACK_ROUTE: {
                      Route src(_track, r->remoteChannel, r->channels);
                      src.remoteChannel = r->channel;
                      r->track->inRoutes()->push_back(src);
                      flags |= SC_ROUTE;
                      // Is this track an Aux Track or else does it have Aux Tracks routed to it?
                      // Update the other track's aux ref count and all tracks it is connected to.
                      if(_track->auxRefCount())
                      {
                        r->track->updateAuxRoute(_track->auxRefCount(), NULL);
                      }
                      else if(_track->type() == Track::AUDIO_AUX)
                      {
                        r->track->updateAuxRoute(1, NULL);
                      }
                    }
                    break;
                    case Route::MIDI_PORT_ROUTE:
                    case Route::JACK_ROUTE:
                    case Route::MIDI_DEVICE_ROUTE:
                    break;
                  }
            }      
      }
      chainTrackParts(_track);

      // Be sure to mark the parts as not deleted if they exist in the global copy/paste clone list.
      const PartList* pl = _track->cparts();
      for(ciPart ip = pl->begin(); ip != pl->end(); ++ip) 
      {
        for(iClone i = MusEGlobal::cloneList.begin(); i != MusEGlobal::cloneList.end(); ++i) 
        {
          if(i->cp == ip->second) 
            i->is_deleted = false;
        }
      }
    }
    break;
    
    case DeleteTrack:
    {
      DEBUG_OPERATIONS(stderr, "PendingOperationItem::executeRTStage DeleteTrack track_list:%p track:%p sec_track_list:%p\n", _track_list, _track, _void_track_list);
      unchainTrackParts(_track);
      if(_void_track_list)
      {
        switch(_track->type())
        {
              case Track::MIDI:
              case Track::DRUM:
              case Track::NEW_DRUM:
                    static_cast<MidiTrackList*>(_void_track_list)->erase(_track);
                    break;
              case Track::WAVE:
                    static_cast<WaveTrackList*>(_void_track_list)->erase(_track);
                    break;
              case Track::AUDIO_OUTPUT:
                    static_cast<OutputList*>(_void_track_list)->erase(_track);
                    break;
              case Track::AUDIO_GROUP:
                    static_cast<GroupList*>(_void_track_list)->erase(_track);
                    break;
              case Track::AUDIO_AUX:
                    static_cast<AuxList*>(_void_track_list)->erase(_track);
                    // Special for aux, make it easier to detect their changes.
                    flags |= SC_AUX;
                    break;
              case Track::AUDIO_INPUT:
                    static_cast<InputList*>(_void_track_list)->erase(_track);
                    break;
              case Track::AUDIO_SOFTSYNTH:
                    {
                      static_cast<SynthIList*>(_void_track_list)->erase(_track);
                      
                      // Change all ports which used the instrument.
                      // FIXME TODO: We want to make this undoable but ATM a few other things can
                      //  set the instrument without an undo operation so the undo sequence would
                      //  not be correct. So we don't have much choice but to just reset for now.
                      // Still, everything else is in place for undoable setting of instrument...
                      const SynthI* si = static_cast<const SynthI*>(_track);
                      for(int port = 0; port < MusECore::MIDI_PORTS; ++port)
                      {
                        MidiPort* mp = &MusEGlobal::midiPorts[port];
                        if(mp->instrument() != si)
                          continue;
                        // Just revert to GM.
                        mp->setInstrument(registerMidiInstrument("GM"));
                        // Flag the port to send initializations next time it bothers to check.
                        mp->clearInitSent();
                        flags |= SC_MIDI_INSTRUMENT;
                      }
                    }     
                    break;
              default:
                    ERROR_OPERATIONS(stderr, "PendingOperationItem::executeRTStage DeleteTrack: Unknown track type %d\n", _track->type());
                    return flags;
        }
      }
      _track_list->erase(_track);
      flags |= SC_TRACK_REMOVED;

      // Remove routes:
      if(_track->type() == Track::AUDIO_OUTPUT) 
      {
            // Clear the track's jack ports
            for(int ch = 0; ch < _track->channels(); ++ch)
            {
              ((AudioOutput*)_track)->setJackPort(ch, 0);
              flags |= SC_ROUTE;
            }
            
            // Clear the track's output routes' jack ports
            RouteList* orl = _track->outRoutes();
            for(iRoute r = orl->begin(); r != orl->end(); ++r)
            {
              if(r->type != Route::JACK_ROUTE)
                continue;
              r->jackPort = 0;
              flags |= SC_ROUTE;
            }
      }
      else if(_track->type() == Track::AUDIO_INPUT) 
      {
            // Clear the track's jack ports
            for(int ch = 0; ch < _track->channels(); ++ch)
            {
              ((AudioInput*)_track)->setJackPort(ch, 0);
              flags |= SC_ROUTE;
            }
            
            // Clear the track's input routes' jack ports
            RouteList* irl = _track->inRoutes();
            for(iRoute r = irl->begin(); r != irl->end(); ++r)
            {
              if(r->type != Route::JACK_ROUTE)
                continue;
              r->jackPort = 0;
              flags |= SC_ROUTE;
            }
      }
      
      if(_track->isMidiTrack())
      {
            // Remove any port output routes to this track
            const RouteList* rl = _track->inRoutes();
            for(ciRoute r = rl->begin(); r != rl->end(); ++r)
            {
                  switch(r->type)
                  {
                    case Route::MIDI_PORT_ROUTE:  {
                      Route src(_track, r->channel);
                      MusEGlobal::midiPorts[r->midiPort].outRoutes()->removeRoute(src);
                      flags |= SC_ROUTE; }
                    break;
                    case Route::TRACK_ROUTE:
                    case Route::JACK_ROUTE:
                    case Route::MIDI_DEVICE_ROUTE:
                    break;
                  }
            }
            // Remove any port input routes from this track
            rl = _track->outRoutes();
            for(ciRoute r = rl->begin(); r != rl->end(); ++r)
            {
                  switch(r->type)
                  {
                    case Route::MIDI_PORT_ROUTE:  {
                      Route src(_track, r->channel);
                      MusEGlobal::midiPorts[r->midiPort].inRoutes()->removeRoute(src);
                      flags |= SC_ROUTE; }
                    break;
                    case Route::TRACK_ROUTE:
                    case Route::JACK_ROUTE:
                    case Route::MIDI_DEVICE_ROUTE:
                    break;
                  }
            }      
      }
      else 
      {
            // Remove other tracks' output routes to this track
            const RouteList* rl = _track->inRoutes();
            for(ciRoute r = rl->begin(); r != rl->end(); ++r)
            {
                  switch(r->type)
                  {
                    case Route::TRACK_ROUTE: {
                      Route src(_track, r->remoteChannel, r->channels);
                      src.remoteChannel = r->channel;
                      r->track->outRoutes()->removeRoute(src);
                      flags |= SC_ROUTE; 
                      // Is the source an Aux Track or else does it have Aux Tracks routed to it?
                      // Update this track's aux ref count.
                      if(r->track->auxRefCount())
                      {
                        _track->updateAuxRoute(-r->track->auxRefCount(), NULL);
                      }
                      else if(r->track->type() == Track::AUDIO_AUX)
                      {
                        _track->updateAuxRoute(-1, NULL);
                      }
                    }
                    break;
                    case Route::MIDI_PORT_ROUTE:
                    case Route::JACK_ROUTE:
                    case Route::MIDI_DEVICE_ROUTE:
                    break;
                  }
            }
            // Remove other tracks' input routes from this track
            rl = _track->outRoutes();
            for(ciRoute r = rl->begin(); r != rl->end(); ++r)
            {
                  switch(r->type)
                  {
                    case Route::TRACK_ROUTE: {
                      Route src(_track, r->remoteChannel, r->channels);
                      src.remoteChannel = r->channel;
                      r->track->inRoutes()->removeRoute(src);
                      flags |= SC_ROUTE;
                      // Is this track an Aux Track or else does it have Aux Tracks routed to it?
                      // Update the other track's aux ref count and all tracks it is connected to.
                      if(_track->auxRefCount())
                      {
                        r->track->updateAuxRoute(-_track->auxRefCount(), NULL);
                      }
                      else if(_track->type() == Track::AUDIO_AUX)
                      {
                        r->track->updateAuxRoute(-1, NULL);
                      }
                    }
                    break;
                    case Route::MIDI_PORT_ROUTE:
                    case Route::JACK_ROUTE:
                    case Route::MIDI_DEVICE_ROUTE:
                    break;
                  }
            }      
      }

      // Be sure to mark the parts as deleted if they exist in the global copy/paste clone list.
      const PartList* pl = _track->cparts();
      for(ciPart ip = pl->begin(); ip != pl->end(); ++ip) 
      {
        for(iClone i = MusEGlobal::cloneList.begin(); i != MusEGlobal::cloneList.end(); ++i) 
        {
          if(i->cp == ip->second) 
            i->is_deleted = true;
        }
      }
    }  
    break;
    
    case MoveTrack:
    {
      DEBUG_OPERATIONS(stderr, "PendingOperationItem::executeRTStage MoveTrack from:%d to:%d\n", _from_idx, _to_idx);
      const int sz = _track_list->size();
      if(_from_idx >= sz)
      {
        ERROR_OPERATIONS(stderr, "MusE error: PendingOperationItem::executeRTStage MoveTrack from index out of range:%d\n", _from_idx);
        return flags;
      }
      ciTrack fromIt = _track_list->cbegin() + _from_idx;
      Track* track = *fromIt;
      _track_list->erase(fromIt);
      ciTrack toIt = (_to_idx >= sz) ? _track_list->cend() : _track_list->cbegin() + _to_idx;
      _track_list->insert(toIt, track);
      flags |= SC_TRACK_MOVED;
    }
    break;
    
    case ModifyTrackName:
      DEBUG_OPERATIONS(stderr, "PendingOperationItem::executeRTStage ModifyTrackName track:%p new_val:%s\n", _track, _name->toLocal8Bit().data());
      _track->setName(*_name);
      flags |= (SC_TRACK_MODIFIED | SC_MIDI_TRACK_PROP);
      // If it's an aux track, notify aux UI controls to reload, or change their names etc.
      if(_track->type() == Track::AUDIO_AUX)
        flags |= SC_AUX;
    break;
    
    case SetTrackRecord:
    {
      DEBUG_OPERATIONS(stderr, "PendingOperationItem::executeRTStage SetTrackRecord track:%p new_val:%d\n", _track, _boolA);
      const bool mon = _track->setRecordFlag2AndCheckMonitor(_boolA);
      flags |= SC_RECFLAG;
      if(mon)
        flags |= SC_TRACK_REC_MONITOR;
    }
    break;
    
    case SetTrackMute:
      DEBUG_OPERATIONS(stderr, "PendingOperationItem::executeRTStage SetTrackMute track:%p new_val:%d\n", _track, _boolA);
      _track->setMute(_boolA);
      flags |= SC_MUTE;
    break;
    
    case SetTrackSolo:
      DEBUG_OPERATIONS(stderr, "PendingOperationItem::executeRTStage SetTrackSolo track:%p new_val:%d\n", _track, _boolA);
      _track->setSolo(_boolA);
      flags |= SC_SOLO;
    break;
    
    case SetTrackRecMonitor:
#ifdef _PENDING_OPS_DEBUG_
      fprintf(stderr, "PendingOperationItem::executeRTStage SetTrackRecMonitor track:%p new_val:%d\n", _track, _boolA);
#endif      
      _track->setRecMonitor(_boolA);
      flags |= SC_TRACK_REC_MONITOR;
    break;
    
    case SetTrackOff:
#ifdef _PENDING_OPS_DEBUG_
      fprintf(stderr, "PendingOperationItem::executeRTStage SetTrackOff track:%p new_val:%d\n", _track, _boolA);
#endif      
      _track->setOff(_boolA);
      flags |= SC_MUTE;
    break;
    
    
    case AddPart:
      DEBUG_OPERATIONS(stderr, "PendingOperationItem::executeRTStage AddPart part:%p\n", _part);
      _part_list->add(_part);
      _part->rechainClone();
      // Be sure to mark the part as not deleted if it exists in the global copy/paste clone list.
      for(iClone i = MusEGlobal::cloneList.begin(); i != MusEGlobal::cloneList.end(); ++i) 
      {
        if(i->cp == _part) 
          i->is_deleted = false;
      }
      flags |= SC_PART_INSERTED;
      // If the part has events, then treat it as if they were inserted with separate AddEvent operations.
      // Even if some will be inserted later in this operations group with actual separate AddEvent operations,
      //  that's an SC_EVENT_INSERTED anyway, so hopefully no harm.
      if(!_part->events().empty())
        flags |= SC_EVENT_INSERTED;
    break;
    
    case DeletePart:
    {
      DEBUG_OPERATIONS(stderr, "PendingOperationItem::executeRTStage DeletePart part:%p\n", _iPart->second);
      Part* p = _iPart->second;
      _part_list->erase(_iPart);
      p->unchainClone();
      // Be sure to mark the part as deleted if it exists in the global copy/paste clone list.
      for(iClone i = MusEGlobal::cloneList.begin(); i != MusEGlobal::cloneList.end(); ++i) 
      {
        if(i->cp == p) 
          i->is_deleted = true;
      }
      flags |= SC_PART_REMOVED;
      // If the part had events, then treat it as if they were removed with separate DeleteEvent operations.
      // Even if they will be deleted later in this operations group with actual separate DeleteEvent operations,
      //  that's an SC_EVENT_REMOVED anyway, so hopefully no harm. This fixes a problem with midi controller canvas
      //  not updating after such a 'delete part with events, no separate AddEvents were used when creating the part'.
      if(!p->events().empty())
        flags |= SC_EVENT_REMOVED;
    }
    break;

    case ModifyPartLength:
      DEBUG_OPERATIONS(stderr, "PendingOperationItem::executeRTStage ModifyPartLength part:%p old_val:%d new_val:%u\n", _part, _part->lenValue(), _posLenVal);
      //_part->type() == Pos::FRAMES ? _part->setLenFrame(_posLenVal) : _part->setLenTick(_posLenVal);
      _part->setLenValue(_posLenVal);
      flags |= SC_PART_MODIFIED;
    break;
    
    case MovePart:
      DEBUG_OPERATIONS(stderr, "PendingOperationItem::executeRTStage MovePart part:%p track:%p new_pos:%u\n", _part, _track, _posLenVal);
      if(_track)
      {
        if(_part->track() && _iPart != _part->track()->parts()->end())
        {
          _part->track()->parts()->erase(_iPart);
          flags |= SC_PART_REMOVED;
        }
        _part->setTrack(_track);
        //_part->setTick(_posLenVal);
        _part->setPosValue(_posLenVal);
        _track->parts()->add(_part);
        flags |= SC_PART_INSERTED;
      }
      else
      {
        //_part->setTick(_posLenVal);
        _part->setPosValue(_posLenVal);
      }
      flags |= SC_PART_MODIFIED;
    break;

    case SelectPart:
#ifdef _PENDING_OPS_DEBUG_
      fprintf(stderr, "PendingOperationItem::executeRTStage SelectPart part:%p select:%u\n", _part, _posLenVal);
#endif      
      if(_part)
        _part->setSelected(_posLenVal);
      
      flags |= SC_PART_SELECTION;
    break;

    case ModifyPartName:
      DEBUG_OPERATIONS(stderr, "PendingOperationItem::executeRTStage ModifyPartName part:%p new_val:%s\n", _part, _name->toLocal8Bit().data());
      _part->setName(*_name);
      flags |= SC_PART_MODIFIED;
    break;
    
    
    case AddEvent:
#ifdef _PENDING_OPS_DEBUG_
      DEBUG_OPERATIONS(stderr, "PendingOperationItem::executeRTStage AddEvent pre:    ");
      _ev.dump();
#endif
      _part->addEvent(_ev);
#ifdef _PENDING_OPS_DEBUG_
      DEBUG_OPERATIONS(stderr, "PendingOperationItem::executeRTStage AddEvent post:   ");
      _ev.dump();
#endif
      flags |= SC_EVENT_INSERTED;
    break;
    
    case DeleteEvent:
#ifdef _PENDING_OPS_DEBUG_
      DEBUG_OPERATIONS(stderr, "PendingOperationItem::executeRTStage DeleteEvent pre:    ");
      _ev.dump();
#endif
      _part->nonconst_events().erase(_iev);
#ifdef _PENDING_OPS_DEBUG_
      DEBUG_OPERATIONS(stderr, "PendingOperationItem::executeRTStage DeleteEvent post:   ");
      _ev.dump();
#endif
      flags |= SC_EVENT_REMOVED;
    break;
    
    case SelectEvent:
#ifdef _PENDING_OPS_DEBUG_
      fprintf(stderr, "PendingOperationItem::executeRTStage SelectEvent part:%p select:%d\n", _part, _intA);
#endif
      // Make sure we let song handle this important job, it selects corresponding events in clone parts.
      MusEGlobal::song->selectEvent(_ev, _part, _intA);
      flags |= SC_SELECTION;
    break;

    
    case AddMidiCtrlValList:
      DEBUG_OPERATIONS(stderr, "PendingOperationItem::executeRTStage AddMidiCtrlValList: mcvll:%p mcvl:%p chan:%d\n", _mcvll, _mcvl, _intA);
      _mcvll->add(_intA, _mcvl);
      flags |= SC_MIDI_CONTROLLER_ADD;
    break;
    case AddMidiCtrlVal:
      DEBUG_OPERATIONS(stderr, "PendingOperationItem::executeRTStage AddMidiCtrlVal: mcvl:%p part:%p tick:%u val:%d\n", _mcvl, _part, _posLenVal, _intB);
      // Do not attempt to add cached events which are outside of the part.
      // Or to muted parts, or muted tracks, or 'off' tracks.
      if(_posLenVal >= _part->posValue() &&
         _posLenVal < _part->posValue() + _part->lenValue() &&
         !_part->mute() && 
         (!_part->track() || (!_part->track()->isMute() && !_part->track()->off())))
         // FIXME FINDMICHJETZT XTicks!!
        _mcvl->insert(MidiCtrlValListInsertPair_t(_posLenVal, MidiCtrlVal(_part, _intB)));
      // No song changed flags are required to be set here.
    break;
    case DeleteMidiCtrlVal:
      DEBUG_OPERATIONS(stderr, "PendingOperationItem::executeRTStage DeleteMidiCtrlVal: mcvl:%p tick:%u part:%p val:%d\n", 
                       _mcvl, _imcv->first, _imcv->second.part, _imcv->second.val);
      _mcvl->erase(_imcv);
      // No song changed flags are required to be set here.
    break;
    case ModifyMidiCtrlVal:
      DEBUG_OPERATIONS(stderr, "PendingOperationItem::executeRTStage ModifyMidiCtrlVal: part:%p old_val:%d new_val:%d\n", 
                       _imcv->second.part, _imcv->second.val, _intA);
      _imcv->second.val = _intA;
    break;
    
    
    case ModifyAudioCtrlValList:
    {
      DEBUG_OPERATIONS(stderr, "PendingOperationItem::executeRTStage ModifyAudioCtrlValList: old ctrl_l:%p new ctrl_l:%p\n", _iCtrlList->second, _aud_ctrl_list);
      CtrlList* orig = _iCtrlList->second;
      _iCtrlList->second = _aud_ctrl_list;
      // Transfer the original pointer back to _aud_ctrl_list so it can be deleted in the non-RT stage.
      _aud_ctrl_list = orig;
      flags |= SC_AUDIO_CONTROLLER_LIST;
    }
    break;
    case AddAudioCtrlVal:
      DEBUG_OPERATIONS(stderr, "PendingOperationItem::executeRTStage AddAudioCtrlVal: ctrl_l:%p frame:%u val:%f\n", 
              _aud_ctrl_list, _posLenVal, _ctl_dbl_val);
      //_aud_ctrl_list->insert(CtrlListInsertPair_t(_posLenVal, CtrlVal(_posLenVal, _ctl_dbl_val)));
      // Add will replace if found.
      _aud_ctrl_list->add(_posLenVal, _ctl_dbl_val);
      flags |= SC_AUDIO_CONTROLLER;
    break;
    case DeleteAudioCtrlVal:
      DEBUG_OPERATIONS(stderr, "PendingOperationItem::executeRTStage DeleteAudioCtrlVal: ctrl_l:%p ctrl_num:%d frame:%d val:%f\n", 
                       _aud_ctrl_list, _aud_ctrl_list->id(), _iCtrl->first, _iCtrl->second.val);
      _aud_ctrl_list->erase(_iCtrl);
      flags |= SC_AUDIO_CONTROLLER;
    break;
    case ModifyAudioCtrlVal:
      DEBUG_OPERATIONS(stderr, "PendingOperationItem::executeRTStage ModifyAudioCtrlVal: frame:%u old_val:%f new_val:%f\n", 
                       _iCtrl->first, _iCtrl->second.val, _ctl_dbl_val);
      // If the frame is the same, just change the value.
      if(_iCtrl->second.frame == _posLenVal)
      {
        _iCtrl->second.val = _ctl_dbl_val;
      }
      // Otherwise erase + add is required.
      else
      {
        _aud_ctrl_list->erase(_iCtrl);
        _aud_ctrl_list->insert(CtrlListInsertPair_t(_posLenVal, CtrlVal(_posLenVal, _ctl_dbl_val)));
      }
      flags |= SC_AUDIO_CONTROLLER;
    break;
    
    
    case AddTempo:
      DEBUG_OPERATIONS(stderr, "PendingOperationItem::executeRTStage AddTempo: tempolist:%p tempo:%p %d tick:%u\n", 
                       _tempo_list, _tempo_event, _tempo_event->tempo, _tempo_event->tick);
      _tempo_list->add(_posLenVal, _tempo_event, false);  // Defer normalize until end of stage 2.
      flags |= SC_TEMPO;
    break;
    
    case DeleteTempo:
      {
        DEBUG_OPERATIONS(stderr, "PendingOperationItem::executeRTStage DeleteTempo: tempolist:%p event:%p: tick:%u tempo:%d\n", 
                         _tempo_list, _iTEvent->second, _iTEvent->second->tick,  _iTEvent->second->tempo);
        _tempo_list->del(_iTEvent, false); // Defer normalize until end of stage 2.
        flags |= SC_TEMPO;
      }
    break;
    
    case ModifyTempo:
      DEBUG_OPERATIONS(stderr, "PendingOperationItem::executeRTStage ModifyTempo: tempolist:%p event:%p: tick:%u old_tempo:%d new_tempo:%d\n", 
                       _tempo_list, _iTEvent->second, _iTEvent->second->tick,  _iTEvent->second->tempo, _intA);
      _iTEvent->second->tempo = _intA;
      flags |= SC_TEMPO;
    break;
    
    case SetStaticTempo:
#ifdef _PENDING_OPS_DEBUG_
      fprintf(stderr, "PendingOperationItem::executeRTStage SetStaticTempo: tempolist:%p new_tempo:%d\n", _tempo_list, _intA);
#endif      
      _tempo_list->setStaticTempo(_intA);
      flags |= SC_TEMPO;
    break;
    
    case SetGlobalTempo:
      DEBUG_OPERATIONS(stderr, "PendingOperationItem::executeRTStage SetGlobalTempo: tempolist:%p new_tempo:%d\n", _tempo_list, _intA);
      _tempo_list->setGlobalTempo(_intA);
      flags |= SC_TEMPO;
    break;

    
    case AddSig:
      DEBUG_OPERATIONS(stderr, "PendingOperationItem::executeRTStage AddSig: siglist:%p sig:%p %d/%d tick:%u\n", 
                       _sig_list, _sig_event, _sig_event->sig.z, _sig_event->sig.n, _sig_event->tick);
      _sig_list->add(_posLenVal, _sig_event, false);  // Defer normalize until end of stage 2.
      flags |= SC_SIG;
    break;
    
    case DeleteSig:
      {
        DEBUG_OPERATIONS(stderr, "PendingOperationItem::executeRTStage DeleteSig: siglist:%p event:%p: tick:%u sig:%d/%d\n", 
                         _sig_list, _iSigEvent->second, _iSigEvent->second->tick,  _iSigEvent->second->sig.z, _iSigEvent->second->sig.n);
        _sig_list->del(_iSigEvent, false); // Defer normalize until end of stage 2.
        flags |= SC_SIG;
      }
    break;
    
    case ModifySig:
      DEBUG_OPERATIONS(stderr, "PendingOperationItem::executeRTStage ModifySig: siglist:%p event:%p: tick:%u old_sig:%d/%d new_sig:%d/%d\n", 
                       _sig_list, _iSigEvent->second, _iSigEvent->second->tick,  _iSigEvent->second->sig.z, _iSigEvent->second->sig.n, _intA, _intB);
      _iSigEvent->second->sig.z = _intA;
      _iSigEvent->second->sig.n = _intB;
      flags |= SC_SIG;
    break;
    
    
    case AddKey:
      DEBUG_OPERATIONS(stderr, "PendingOperationItem::executeRTStage AddKey: keylist:%p key:%d tick:%u\n", _key_list, _intB, _posLenVal);
      _key_list->add(KeyEvent(key_enum(_intB), _posLenVal)); 
      flags |= SC_KEY;
    break;
    
    case DeleteKey:
      {
        DEBUG_OPERATIONS(stderr, "PendingOperationItem::executeRTStage DeleteKey: keylist:%p key:%d tick:%u\n",
                         _key_list, _iKeyEvent->second.key, _iKeyEvent->second.tick);
        _key_list->del(_iKeyEvent);
        flags |= SC_KEY;
      }
    break;
    
    case ModifyKey:
      DEBUG_OPERATIONS(stderr, "PendingOperationItem::executeRTStage ModifyKey: keylist:%p old_key:%d new_key:%d tick:%u\n", 
                       _key_list, _iKeyEvent->second.key, _intA, _iKeyEvent->second.tick);
      _iKeyEvent->second.key = key_enum(_intA);
      flags |= SC_KEY;
    break;

    
    case ModifyStretchListRatio:
      DEBUG_OPERATIONS(stderr, "PendingOperationItem::executeRTStage ModifyStretchListRatio: stretchType:%d stretchlist:%p new_ratio:%f\n", 
                       _stretch_type, _stretch_list, _audio_converter_value);
      // Defer normalize until end of stage 2.
      _stretch_list->setRatio(StretchListItem::StretchEventType(_stretch_type), _audio_converter_value, false);
      flags |= SC_AUDIO_STRETCH;
    break;
    
    case AddStretchListRatioAt:
      DEBUG_OPERATIONS(stderr, "PendingOperationItem::executeRTStage AddStretchListRatioAt: stretchType:%d stretchlist:%p ratio:%f frame:%ld\n", 
                       _stretch_type, _stretch_list, _audio_converter_value, _museFrame);
      // Defer normalize until end of stage 2.
      _stretch_list->addRatioAt(StretchListItem::StretchEventType(_stretch_type), _museFrame, _audio_converter_value, false);
      
      flags |= SC_AUDIO_STRETCH;
    break;
    
    case DeleteStretchListRatioAt:
      DEBUG_OPERATIONS(stderr, "PendingOperationItem::executeRTStage DeleteStretchListRatioAt: stretchlist:%p frame:%ld types:%d\n", 
                        _stretch_list, _iStretchEvent->first, _stretch_type);
      // Defer normalize until end of stage 2.
      _stretch_list->del(_stretch_type, _iStretchEvent, false);
      flags |= SC_AUDIO_STRETCH;
    break;
    
    case ModifyStretchListRatioAt:
      DEBUG_OPERATIONS(stderr, "PendingOperationItem::executeRTStage ModifyStretchListRatioAt: "
                               "stretchType:%d stretchlist:%p frame:%ld new_frame:%ld new_ratio:%f\n", 
                       _stretch_type, _stretch_list, _iStretchEvent->first, _museFrame, _audio_converter_value);
      
      // If the frame is the same, just change the value.
      if(_iStretchEvent->first == _museFrame)
        // Defer normalize until end of stage 2.
        _stretch_list->setRatioAt(StretchListItem::StretchEventType(_stretch_type), _iStretchEvent, _audio_converter_value, false);
      // Otherwise erase + add is required.
      else
      {
        // Defer normalize until end of stage 2.
        _stretch_list->del(_stretch_type, _iStretchEvent, false);
        _stretch_list->add(StretchListItem::StretchEventType(_stretch_type), _museFrame, _audio_converter_value, false);
      }
      
      flags |= SC_AUDIO_STRETCH;
    break;
    
    
    case ModifySongLength:
      DEBUG_OPERATIONS(stderr, "PendingOperationItem::executeRTStage ModifySongLength: len:%d\n", _posLenVal);
      MusEGlobal::song->setLen(_posLenVal, false); // false = Do not emit update signals here !
      flags |= SC_EVERYTHING;
    break;
    
    case EnableAllAudioControllers:
    {
      DEBUG_OPERATIONS(stderr, "PendingOperationItem::executeRTStage EnableAllAudioControllers\n");
      TrackList* tl = MusEGlobal::song->tracks();
      for (iTrack it = tl->begin(); it != tl->end(); ++it)
      {
        Track* t = *it;
        if(t->isMidiTrack())
          continue;
        AudioTrack *at = static_cast<AudioTrack*>(t);
        // Re-enable all track and plugin controllers, and synth controllers if applicable.
        at->enableAllControllers();
        flags |= SC_AUDIO_CONTROLLER;
      }
    }
    break;
    
    case GlobalSelectAllEvents:
    {
#ifdef _PENDING_OPS_DEBUG_
      fprintf(stderr, "PendingOperationItem::executeRTStage GlobalSelectAllEvents\n");
#endif
      for (iTrack it = _track_list->begin(); it != _track_list->end(); ++it)
      {
        //Track* t = *it;
        //if(t->isMidiTrack())
        //  continue;
        if((*it)->selectEvents(_boolA))
          flags |= SC_SELECTION;
      }
    }
    break;
    
    case ModifyAudioSamples:
    {
#ifdef _PENDING_OPS_DEBUG_
      fprintf(stderr, "PendingOperationItem::executeRTStage ModifyAudioSamples: "
                      "audioSamplesPointer:%p newAudioSamples:%p audioSamplesLen:%p newAudioSamplesLen:%d\n",
              _audioSamplesPointer, _newAudioSamples, _audioSamplesLen, _newAudioSamplesLen);
#endif      
      if(_audioSamplesPointer)
      {
        float* orig = *_audioSamplesPointer;
        *_audioSamplesPointer = _newAudioSamples;
        // Transfer the original pointer back to _audioSamplesPointer so it can be deleted in the non-RT stage.
        _newAudioSamples = orig;
      }
      
      if(_audioSamplesLen)
        *_audioSamplesLen = _newAudioSamplesLen;
      
      // Currently no flags for this.
      //flags |= SC_;
    }
    break;

    case SwitchMetronomeSettings:
    {
#ifdef _PENDING_OPS_DEBUG_
      fprintf(stderr, "PendingOperationItem::executeRTStage SwitchMetronomeSettings: settings:%p val:%d\n", _bool_pointer, _boolA);
#endif      
      *_bool_pointer = _boolA;
      flags |= SC_METRONOME;
    }
    break;

    case ModifyMetronomeAccentMap:
    {
#ifdef _PENDING_OPS_DEBUG_
      fprintf(stderr, "PendingOperationItem::executeRTStage ModifyMetronomeAccentMap: old map:%p new map:%p\n", _metroAccentsMap, _newMetroAccentsMap);
#endif      
      MetroAccentsMap* orig = *_metroAccentsMap;
      *_metroAccentsMap = _newMetroAccentsMap;
      // Transfer the original pointer back to _newMetroAccentsMap so it can be deleted in the non-RT stage.
      _newMetroAccentsMap = orig;
      flags |= SC_METRONOME;
    }
    break;

    case SetExternalSyncFlag:
    {
#ifdef _PENDING_OPS_DEBUG_
      fprintf(stderr, "PendingOperationItem::executeRTStage SetExternalSyncFlag: pointer:%p val:%d\n", _bool_pointer, _boolA);
#endif      
      *_bool_pointer = _boolA;
      flags |= SC_EXTERNAL_MIDI_SYNC;
    }
    break;

    case SetUseJackTransport:
    {
#ifdef _PENDING_OPS_DEBUG_
      fprintf(stderr, "PendingOperationItem::executeRTStage SetUseJackTransport: pointer:%p val:%d\n", _bool_pointer, _boolA);
#endif      
      *_bool_pointer = _boolA;
      flags |= SC_USE_JACK_TRANSPORT;
    }
    break;

    case SetUseMasterTrack:
    {
#ifdef _PENDING_OPS_DEBUG_
      fprintf(stderr, "PendingOperationItem::executeRTStage SetUseMasterTrack: pointer:%p val:%d\n", _tempo_list, _boolA);
#endif      
      _tempo_list->setMasterFlag(0, _boolA);
      flags |= SC_MASTER;
    }
    break;

    case Uninitialized:
    break;
    
    default:
      ERROR_OPERATIONS(stderr, "PendingOperationItem::executeRTStage unknown type %d\n", _type);
    break;
  }
  return flags;
}

SongChangedStruct_t PendingOperationItem::executeNonRTStage()
{
  SongChangedStruct_t flags = 0;
  switch(_type)
  {
    case AddRoute:
      if(MusEGlobal::song->connectJackRoutes(_src_route, _dst_route))
        flags |= SC_ROUTE;
    break;
    
    case DeleteRoute:
      if(MusEGlobal::song->connectJackRoutes(_src_route, _dst_route, true))
        flags |= SC_ROUTE;
    break;

    case DeleteTempo:
      {
        DEBUG_OPERATIONS(stderr, "PendingOperationItem::executeNonRTStage DeleteTempo: tempolist:%p event:%p:\n", 
                         _tempo_list, _tempo_event);
        if(_tempo_event)
        {  
          delete _tempo_event;
          _tempo_event = 0;
        }
      }
    break;
    
    case DeleteSig:
      {
        DEBUG_OPERATIONS(stderr, "PendingOperationItem::executeNonRTStage DeleteSig: siglist:%p event:%p:\n", 
                         _sig_list, _sig_event);
        if(_sig_event)
        {  
          delete _sig_event;
          _sig_event = 0;
        }
      }
    break;
    
    case ModifyLocalAudioConverterSettings:
      // At this point these are the original pointers that were replaced. Delete the original objects now.
      if(_audio_converter_settings)
        delete _audio_converter_settings;
    break;

    case ModifyLocalAudioConverter:
      // At this point these are the original pointers that were replaced. Delete the original objects now.
      if(_audio_converter)
        delete _audio_converter;
      if(_audio_converter_ui)
        delete _audio_converter_ui;
    break;

    case SetAudioConverterOfflineMode:
      // At this point this is the original pointer that were replaced. Delete the original object now.
      if(_audio_converter)
        delete _audio_converter;
    break;

    case ModifyDefaultAudioConverterSettings:
      // At this point this is the original pointer that was replaced. Delete the original object now.
      if(_audio_converter_settings)
        delete _audio_converter_settings;
    break;
    
    case ReplaceMidiInstrument:
      // At this point _midi_instrument is the original instrument that was replaced. Delete it now.
      if(_midi_instrument)
        delete _midi_instrument;
    break;

    case ModifyAudioCtrlValList:
      // At this point _aud_ctrl_list is the original list that was replaced. Delete it now.
      if(_aud_ctrl_list)
        delete _aud_ctrl_list;
    break;

    case ModifyTrackDrumMapItem:
      // Discard the operation, it has already completed.
      if(_drum_map_track_operation)
        delete _drum_map_track_operation;
    break;

    case ReplaceTrackDrumMapPatchList:
      // Discard the operation, it has already completed.
      if(_drum_map_track_patch_operation)
      {
        // At this point _workingItemPatchList is the original list that was replaced. Delete it now.
        if(_drum_map_track_patch_replace_operation->_workingItemPatchList)
          delete _drum_map_track_patch_replace_operation->_workingItemPatchList;

        delete _drum_map_track_patch_replace_operation;
      }
    break;

    case RemapDrumControllers:
      // Discard the operation, it has already completed.
      if(_midi_ctrl_val_remap_operation)
      {
        // At this point _midiCtrlValLists2bDeleted contains the original lists that were replaced. Delete them now.
        for(iMidiCtrlValLists2bDeleted_t imvld = _midi_ctrl_val_remap_operation->_midiCtrlValLists2bDeleted.begin();
            imvld != _midi_ctrl_val_remap_operation->_midiCtrlValLists2bDeleted.end(); ++imvld)
          delete *imvld;

        delete _midi_ctrl_val_remap_operation;
      }
    break;

    case ModifyAudioSamples:
      // At this point _newAudioSamples points to the original memory that was replaced. Delete it now.
      if(_newAudioSamples)
        delete _newAudioSamples;
    break;

    case ModifyMetronomeAccentMap:
      // At this point _newMetroAccentsMap is the original list that was replaced. Delete it now.
      if(_newMetroAccentsMap)
        delete _newMetroAccentsMap;
    break;

    default:
    break;
  }
  return flags;
}

SongChangedStruct_t PendingOperationList::executeRTStage()
{
  DEBUG_OPERATIONS(stderr, "PendingOperationList::executeRTStage executing...\n");
  for(iPendingOperation ip = begin(); ip != end(); ++ip)
    _sc_flags |= ip->executeRTStage();
  
  // To avoid doing this item by item, do it here.
  if(_sc_flags & (SC_TRACK_INSERTED | SC_TRACK_REMOVED | SC_ROUTE))
  {
    MusEGlobal::song->updateSoloStates();
    _sc_flags |= SC_SOLO;
  } 
  
  // To avoid doing this item by item, do it here.
  StretchList* sl;
  for(iPendingOperation ip = begin(); ip != end(); ++ip)
  {
    const PendingOperationItem& poi = *ip;
    switch(poi._type)
    {
      //case PendingOperationItem::AddSamplerateRatioAt:
      //case PendingOperationItem::DeleteSamplerateRatioAt:
      //case PendingOperationItem::ModifySamplerateRatioAt:
      case PendingOperationItem::AddStretchListRatioAt:
      case PendingOperationItem::DeleteStretchListRatioAt:
      case PendingOperationItem::ModifyStretchListRatioAt:
      case PendingOperationItem::ModifyStretchListRatio:
        sl = poi._stretch_list;
        if(sl && !sl->isNormalized())
        {
          sl->normalizeListFrames();
          _sc_flags |= SC_AUDIO_STRETCH;
        }
      break;
      
      default:
      break;
    }
  }

  return _sc_flags;
}

SongChangedStruct_t PendingOperationList::executeNonRTStage()
{
  DEBUG_OPERATIONS(stderr, "PendingOperationList::executeNonRTStage executing...\n");
  for(iPendingOperation ip = begin(); ip != end(); ++ip)
    _sc_flags |= ip->executeNonRTStage();
  return _sc_flags;
}

void PendingOperationList::clear()
{
  _sc_flags = 0;
  _map.clear();
  std::list<PendingOperationItem>::clear();  
  DEBUG_OPERATIONS(stderr, "PendingOperationList::clear * post map size:%d list size:%d\n", (int)_map.size(), (int)size());
}

bool PendingOperationList::add(PendingOperationItem op)
{
  unsigned int t = op.getIndex();

  switch(op._type)
  {
    // For these special allocation ops, searching has already been done before hand. Just add them.
    case PendingOperationItem::AddMidiCtrlValList:
    case PendingOperationItem::AddTempo:
    case PendingOperationItem::AddSig:
    {
      iPendingOperation iipo = insert(end(), op);
      _map.insert(std::pair<unsigned int, iPendingOperation>(t, iipo));
      return true;
    }
    break;
    
    default:
    break;
  }
  
  iPendingOperationSortedRange r = _map.equal_range(t);
  iPendingOperationSorted ipos = r.second;
  while(ipos != r.first)
  {
    --ipos;
    PendingOperationItem& poi = *ipos->second;
    
    switch(op._type)
    {
      case PendingOperationItem::ModifyDefaultAudioConverterSettings:
        if(poi._type == PendingOperationItem::ModifyDefaultAudioConverterSettings && 
           poi._audio_converter_settings == op._audio_converter_settings)
        {
          ERROR_OPERATIONS(stderr, "MusE error: PendingOperationList::add(): Double ModifyDefaultAudioConverterSettings. Ignoring.\n");
          return false;  
        }
      break;
    
      case PendingOperationItem::ModifyLocalAudioConverterSettings:
        if(poi._type == PendingOperationItem::ModifyLocalAudioConverterSettings && *poi._sndFileR == *op._sndFileR &&
           poi._audio_converter_settings == op._audio_converter_settings)
        {
          ERROR_OPERATIONS(stderr, "MusE error: PendingOperationList::add(): Double ModifyLocalAudioConverterSettings. Ignoring.\n");
          return false;  
        }
      break;
    
      case PendingOperationItem::ModifyLocalAudioConverter:
        if(poi._type == PendingOperationItem::ModifyLocalAudioConverter && *poi._sndFileR == *op._sndFileR &&
           poi._audio_converter == op._audio_converter &&
           poi._audio_converter_ui == op._audio_converter_ui)
        {
          ERROR_OPERATIONS(stderr, "MusE error: PendingOperationList::add(): Double ModifyLocalAudioConverter. Ignoring.\n");
          return false;  
        }
      break;
    
      case PendingOperationItem::SetAudioConverterOfflineMode:
        if(poi._type == PendingOperationItem::SetAudioConverterOfflineMode && *poi._sndFileR == *op._sndFileR &&
           poi._audio_converter == op._audio_converter)
        {
          ERROR_OPERATIONS(stderr, "MusE error: PendingOperationList::add(): Double SetAudioConverterOfflineMode. Ignoring.\n");
          return false;  
        }
      break;


      case PendingOperationItem::ModifyTrackDrumMapItem:
        if(poi._type == PendingOperationItem::ModifyTrackDrumMapItem &&
           poi._drum_map_track_operation == op._drum_map_track_operation)
        {
          fprintf(stderr, "MusE error: PendingOperationList::add(): Double ModifyTrackDrumMapItem. Ignoring.\n");
          return false;
        }
      break;

      case PendingOperationItem::ReplaceTrackDrumMapPatchList:
        if(poi._type == PendingOperationItem::ReplaceTrackDrumMapPatchList &&
           poi._drum_map_track_patch_replace_operation == op._drum_map_track_patch_replace_operation)
        {
          fprintf(stderr, "MusE error: PendingOperationList::add(): Double ReplaceTrackDrumMapPatchList. Ignoring.\n");
          return false;
        }
      break;

      case PendingOperationItem::RemapDrumControllers:
        if(poi._type == PendingOperationItem::RemapDrumControllers &&
           poi._midi_ctrl_val_remap_operation == op._midi_ctrl_val_remap_operation)
        {
          fprintf(stderr, "MusE error: PendingOperationList::add(): Double RemapDrumControllers. Ignoring.\n");
          return false;
        }
      break;

      case PendingOperationItem::UpdateDrumMaps:
        if(poi._type == PendingOperationItem::UpdateDrumMaps && poi._midi_port == op._midi_port)
        {
          fprintf(stderr, "MusE error: PendingOperationList::add(): Double UpdateDrumMaps. Ignoring.\n");
          return false;  
        }
      break;
      
      case PendingOperationItem::UpdateSoloStates:
        if(poi._type == PendingOperationItem::UpdateSoloStates && poi._track_list == op._track_list)
        {
          ERROR_OPERATIONS(stderr, "MusE error: PendingOperationList::add(): Double UpdateSoloStates. Ignoring.\n");
          return false;  
        }
      break;
      
      case PendingOperationItem::AddRoute:
        if(poi._type == PendingOperationItem::AddRoute && poi._src_route == op._src_route && poi._dst_route == op._dst_route)
        {
          ERROR_OPERATIONS(stderr, "MusE error: PendingOperationList::add(): Double AddRoute. Ignoring.\n");
          return false;  
        }
      break;
      
      case PendingOperationItem::DeleteRoute:
        if(poi._type == PendingOperationItem::DeleteRoute && poi._src_route == op._src_route && poi._dst_route == op._dst_route)
        {
          ERROR_OPERATIONS(stderr, "MusE error: PendingOperationList::add(): Double DeleteRoute. Ignoring.\n");
          return false;  
        }
      break;
      
      case PendingOperationItem::AddRouteNode:
        if(poi._type == PendingOperationItem::AddRouteNode && poi._route_list == op._route_list && poi._src_route == op._src_route)
        {
          ERROR_OPERATIONS(stderr, "MusE error: PendingOperationList::add(): Double AddRouteNode. Ignoring.\n");
          return false;  
        }
      break;

      case PendingOperationItem::DeleteRouteNode:
        if(poi._type == PendingOperationItem::DeleteRouteNode && poi._route_list == op._route_list && poi._iRoute == op._iRoute)
        {
          ERROR_OPERATIONS(stderr, "MusE error: PendingOperationList::add(): Double DeleteRouteNode. Ignoring.\n");
          return false;  
        }
      break;

      case PendingOperationItem::ModifyRouteNode:
        if(poi._type == PendingOperationItem::ModifyRouteNode && poi._src_route == op._src_route && poi._dst_route_pointer == op._dst_route_pointer)
        {
          ERROR_OPERATIONS(stderr, "MusE error: PendingOperationList::add(): Double ModifyRouteNode. Ignoring.\n");
          return false;  
        }
      break;


      case PendingOperationItem::AddAuxSendValue:
        if(poi._type == PendingOperationItem::AddAuxSendValue && poi._aux_send_value_list == op._aux_send_value_list)  
        {
          // Do nothing. So far.
        }
      break;
        
      case PendingOperationItem::AddMidiInstrument:
        if(poi._type == PendingOperationItem::AddMidiInstrument && poi._midi_instrument_list == op._midi_instrument_list && 
           poi._midi_instrument == op._midi_instrument)  
        {
          ERROR_OPERATIONS(stderr, "MusE error: PendingOperationList::add(): Double AddMidiInstrument. Ignoring.\n");
          return false;  
        }
      break;
        
      case PendingOperationItem::DeleteMidiInstrument:
        if(poi._type == PendingOperationItem::DeleteMidiInstrument && poi._midi_instrument_list == op._midi_instrument_list && 
           poi._iMidiInstrument == op._iMidiInstrument)
        {
          ERROR_OPERATIONS(stderr, "MusE error: PendingOperationList::add(): Double DeleteMidiInstrument. Ignoring.\n");
          return false;  
        }
      break;
        
      case PendingOperationItem::ReplaceMidiInstrument:
        if(poi._type == PendingOperationItem::ReplaceMidiInstrument && poi._midi_instrument_list == op._midi_instrument_list &&
           (poi._midi_instrument == op._midi_instrument || poi._iMidiInstrument == op._iMidiInstrument))
        {
          fprintf(stderr, "MusE error: PendingOperationList::add(): Double ReplaceMidiInstrument. Ignoring.\n");
          return false;
        }
      break;

      case PendingOperationItem::AddMidiDevice:
        if(poi._type == PendingOperationItem::AddMidiDevice && poi._midi_device_list == op._midi_device_list && poi._midi_device == op._midi_device)  
        {
          ERROR_OPERATIONS(stderr, "MusE error: PendingOperationList::add(): Double AddMidiDevice. Ignoring.\n");
          return false;  
        }
      break;
        
      case PendingOperationItem::DeleteMidiDevice:
        if(poi._type == PendingOperationItem::DeleteMidiDevice && poi._midi_device_list == op._midi_device_list && poi._iMidiDevice == op._iMidiDevice)
        {
          ERROR_OPERATIONS(stderr, "MusE error: PendingOperationList::add(): Double DeleteMidiDevice. Ignoring.\n");
          return false;  
        }
      break;
        
      case PendingOperationItem::ModifyMidiDeviceAddress:
        if(poi._type == PendingOperationItem::ModifyMidiDeviceAddress && poi._midi_device == op._midi_device &&
           poi._address_client == op._address_client && poi._address_port == op._address_port)
        {
          ERROR_OPERATIONS(stderr, "MusE error: PendingOperationList::add(): Double ModifyMidiDeviceAddress. Ignoring.\n");
          return false;  
        }
      break;
        
      case PendingOperationItem::ModifyMidiDeviceFlags:
        if(poi._type == PendingOperationItem::ModifyMidiDeviceFlags && poi._midi_device == op._midi_device &&
           poi._rw_flags == op._rw_flags && poi._open_flags == op._open_flags)
        {
          ERROR_OPERATIONS(stderr, "MusE error: PendingOperationList::add(): Double ModifyMidiDeviceFlags. Ignoring.\n");
          return false;  
        }
      break;
        
      case PendingOperationItem::ModifyMidiDeviceName:
        if(poi._type == PendingOperationItem::ModifyMidiDeviceName && poi._midi_device == op._midi_device &&
           poi._name == op._name)
        {
          ERROR_OPERATIONS(stderr, "MusE error: PendingOperationList::add(): Double ModifyMidiDeviceName. Ignoring.\n");
          return false;  
        }
      break;

      case PendingOperationItem::SetInstrument:
        if(poi._type == PendingOperationItem::SetInstrument && poi._midi_port == op._midi_port &&
           poi._midi_instrument == op._midi_instrument)
        {
          fprintf(stderr, "MusE error: PendingOperationList::add(): Double SetInstrument. Ignoring.\n");
          return false;  
        }
      break;

      
      case PendingOperationItem::AddTrack:
        if(poi._type == PendingOperationItem::AddTrack && poi._track_list == op._track_list && poi._track == op._track)  
        {
          // Simply replace the insert point.
          poi._insert_at = op._insert_at;
          // An operation will still take place.
          return true;  
        }
        else if(poi._type == PendingOperationItem::DeleteTrack && poi._track_list == op._track_list && poi._track == op._track)  
        {
          // Delete followed by add is useless. Cancel out the delete + add by erasing the delete command.
          //erase(ipos->second);
          //_map.erase(ipos);
          // No operation will take place.
          //return false;
        }
      break;
      
      case PendingOperationItem::DeleteTrack:
        if(poi._type == PendingOperationItem::DeleteTrack && poi._track_list == op._track_list && poi._track == op._track)  
        {
          ERROR_OPERATIONS(stderr, "MusE error: PendingOperationList::add(): Double DeleteTrack. Ignoring.\n");
          return false;  
        }
        else if(poi._type == PendingOperationItem::AddTrack && poi._track_list == op._track_list && poi._track == op._track)  
        {
          // Add followed by delete is useless. Cancel out the add + delete by erasing the add command.
          //erase(ipos->second);
          //_map.erase(ipos);
          // No operation will take place.
          //return false;
        }
      break;
      
      case PendingOperationItem::MoveTrack:
        if(poi._type == PendingOperationItem::MoveTrack && poi._track == op._track && poi._track_list == op._track_list)  
        {
          // Simply replace the 'to' index.
          poi._to_idx = op._to_idx;
          // An operation will still take place.
          return true;
        }
      break;
        
      case PendingOperationItem::ModifyTrackName:
        if(poi._type == PendingOperationItem::ModifyTrackName && poi._track == op._track &&
           poi._name == op._name)
        {
          ERROR_OPERATIONS(stderr, "MusE error: PendingOperationList::add(): Double ModifyTrackName. Ignoring.\n");
          return false;  
        }
      break;

      case PendingOperationItem::SetTrackRecord:
        if(poi._type == PendingOperationItem::SetTrackRecord && poi._track == op._track)
        {
          if(poi._boolA == op._boolA)  
          {
            ERROR_OPERATIONS(stderr, "MusE error: PendingOperationList::add(): Double SetTrackRecord. Ignoring.\n");
            return false;  
          }
          else
          {
            // On/off followed by off/on is useless. Cancel out the on/off + off/on by erasing the command.
            erase(ipos->second);
            _map.erase(ipos);
            // No operation will take place.
            return false;
          }
        }
      break;
      
      case PendingOperationItem::SetTrackMute:
        if(poi._type == PendingOperationItem::SetTrackMute && poi._track == op._track)
        {
          if(poi._boolA == op._boolA)  
          {
            ERROR_OPERATIONS(stderr, "MusE error: PendingOperationList::add(): Double SetTrackMute. Ignoring.\n");
            return false;  
          }
          else
          {
            // On/off followed by off/on is useless. Cancel out the on/off + off/on by erasing the command.
            erase(ipos->second);
            _map.erase(ipos);
            // No operation will take place.
            return false;
          }
        }
      break;
      
      case PendingOperationItem::SetTrackSolo:
        if(poi._type == PendingOperationItem::SetTrackSolo && poi._track == op._track)
        {
          if(poi._boolA == op._boolA)  
          {
            ERROR_OPERATIONS(stderr, "MusE error: PendingOperationList::add(): Double SetTrackSolo. Ignoring.\n");
            return false;  
          }
          else
          {
            // On/off followed by off/on is useless. Cancel out the on/off + off/on by erasing the command.
            erase(ipos->second);
            _map.erase(ipos);
            // No operation will take place.
            return false;
          }
        }
      break;
      
      case PendingOperationItem::SetTrackRecMonitor:
        if(poi._type == PendingOperationItem::SetTrackRecMonitor && poi._track == op._track)
        {
          if(poi._boolA == op._boolA)  
          {
            fprintf(stderr, "MusE error: PendingOperationList::add(): Double SetTrackRecMonitor. Ignoring.\n");
            return false;  
          }
          else
          {
            // On/off followed by off/on is useless. Cancel out the on/off + off/on by erasing the command.
            erase(ipos->second);
            _map.erase(ipos);
            // No operation will take place.
            return false;
          }
        }
      break;
      
      case PendingOperationItem::SetTrackOff:
        if(poi._type == PendingOperationItem::SetTrackOff && poi._track == op._track)
        {
          if(poi._boolA == op._boolA)  
          {
            fprintf(stderr, "MusE error: PendingOperationList::add(): Double SetTrackOff. Ignoring.\n");
            return false;  
          }
          else
          {
            // On/off followed by off/on is useless. Cancel out the on/off + off/on by erasing the command.
            erase(ipos->second);
            _map.erase(ipos);
            // No operation will take place.
            return false;
          }
        }
      break;
      
      case PendingOperationItem::AddPart:
        if(poi._type == PendingOperationItem::AddPart && poi._part_list == op._part_list && poi._part == op._part)  
        {
          ERROR_OPERATIONS(stderr, "MusE error: PendingOperationList::add(): Double AddPart. Ignoring.\n");
          return false;  
        }
        else if(poi._type == PendingOperationItem::DeletePart && poi._part_list == op._part_list && poi._iPart->second == op._part)  
        {
          // Delete followed by add is useless. Cancel out the delete + add by erasing the delete command.
          erase(ipos->second);
          _map.erase(ipos);
          // No operation will take place.
          return false;
        }
      break;
      
      case PendingOperationItem::DeletePart:
        if(poi._type == PendingOperationItem::DeletePart && poi._part_list == op._part_list && poi._iPart->second == op._iPart->second)  
        {
          ERROR_OPERATIONS(stderr, "MusE error: PendingOperationList::add(): Double DeletePart. Ignoring.\n");
          return false;  
        }
        else if(poi._type == PendingOperationItem::AddPart && poi._part_list == op._part_list && poi._part == op._iPart->second)  
        {
          // Add followed by delete is useless. Cancel out the add + delete by erasing the add command.
          erase(ipos->second);
          _map.erase(ipos);
          // No operation will take place.
          return false;
        }
      break;

      case PendingOperationItem::SelectPart:
        if(poi._type == PendingOperationItem::SelectPart && poi._part == op._part)  
        {
          // Simply replace the value.
          poi._intA = op._intA;
          // An operation will still take place.
          return true;
        }
      break;
      
      case PendingOperationItem::MovePart:
        if(poi._type == PendingOperationItem::MovePart && poi._part == op._part)  
        {
          // Simply replace the values.
          poi._iPart = op._iPart;
          poi._track = op._track;
          poi._posLenVal = op._posLenVal;
          // An operation will still take place.
          return true;
        }
      break;
      
      case PendingOperationItem::ModifyPartName:
        if(poi._type == PendingOperationItem::ModifyPartName && poi._part == op._part &&
           poi._name == op._name)
        {
          ERROR_OPERATIONS(stderr, "MusE error: PendingOperationList::add(): Double ModifyPartName. Ignoring.\n");
          return false;  
        }
      break;
        
      
      case PendingOperationItem::AddEvent:
        if(poi._type == PendingOperationItem::AddEvent && poi._part == op._part && poi._ev == op._ev)  
        {
          ERROR_OPERATIONS(stderr, "MusE error: PendingOperationList::add(): Double AddEvent. Ignoring.\n");
          return false;  
        }
        else if(poi._type == PendingOperationItem::DeleteEvent && poi._part == op._part && poi._iev->second == op._ev)  
        {
          // Delete followed by add is useless. Cancel out the delete + add by erasing the delete command.
          erase(ipos->second);
          _map.erase(ipos);
          // No operation will take place.
          return false;
        }
      break;
      
      case PendingOperationItem::DeleteEvent:
        if(poi._type == PendingOperationItem::DeleteEvent && poi._part == op._part && poi._iev->second == op._iev->second)  
        {
          ERROR_OPERATIONS(stderr, "MusE error: PendingOperationList::add(): Double DeleteEvent. Ignoring.\n");
          return false;  
        }
        else if(poi._type == PendingOperationItem::AddEvent && poi._part == op._part && poi._ev == op._iev->second)  
        {
          // Add followed by delete is useless. Cancel out the add + delete by erasing the add command.
          erase(ipos->second);
          _map.erase(ipos);
          // No operation will take place.
          return false;
        }
      break;

      case PendingOperationItem::SelectEvent:
        if(poi._type == PendingOperationItem::SelectEvent &&
           poi._part == op._part && poi._ev == op._ev)
        {
          // Simply replace the value.
          poi._intA = op._intA;
          // An operation will still take place.
          return true;
        }
      break;
      
      case PendingOperationItem::AddMidiCtrlVal:
        if(poi._type == PendingOperationItem::DeleteMidiCtrlVal && 
           poi._mcvl == op._mcvl && 
           poi._imcv->second.part == op._part &&
           poi._imcv->second.val == op._intB)
        {
          // Delete followed by add is useless. Cancel out the delete + add by erasing the delete command.
          erase(ipos->second);
          _map.erase(ipos);
          // No operation will take place.
          return false;
        }
      break;
      
      case PendingOperationItem::DeleteMidiCtrlVal:
        // Be sure _intB is set.
        if(poi._type == PendingOperationItem::AddMidiCtrlVal && 
           poi._mcvl == op._mcvl && 
           poi._part == op._imcv->second.part &&
           poi._intB == op._imcv->second.val)
        {
          // Add followed by delete is useless. Cancel out the add + delete by erasing the add command.
          erase(ipos->second);
          _map.erase(ipos);
          // No operation will take place.
          return false;
        }
      break;
      
      case PendingOperationItem::ModifyMidiCtrlVal:
// TODO FIXME Finish this
        
           // Be sure _intB/A is set
//         if(poi._type == PendingOperationItem::ModifyMidiCtrlVal &&
//            poi._mcvl == op._mcvl && 
//            poi._imcv->second.part == op._imcv->second.part &&
//            poi._imcv->second.val == op._imcv->second.val)
//         {
//           // Simply replace the value.
//           poi._intA = op._intA;
//           return true;
//         }
//         else if(poi._type == PendingOperationItem::DeleteMidiCtrlVal && poi._mcvl == op._mcvl && poi._imcv->second.part == op._imcv->second.part)
//         {
//           // Transform existing delete command into a modify command.
//           poi._type = PendingOperationItem::ModifyMidiCtrlVal;
//           poi._intA = op._intA; 
//           return true;
//         }
//         else if(poi._type == PendingOperationItem::AddMidiCtrlVal && poi._mcvl == op._mcvl && poi._part == op._imcv->second.part)
//         {
//           // Simply replace the add value with the modify value.
//           poi._intB = op._intA; 
//           return true;
//         }
      break;
      
      
      case PendingOperationItem::ModifyAudioCtrlValList:
        if(poi._type == PendingOperationItem::ModifyAudioCtrlValList && 
          // If attempting to repeatedly modify the same list, or, if progressively modifying (list to list to list etc).
          (poi._iCtrlList->second == op._iCtrlList->second || poi._aud_ctrl_list == op._iCtrlList->second))
        {
          // Simply replace the list.
          poi._aud_ctrl_list = op._aud_ctrl_list; 
          // An operation will still take place.
          return true;
        }
      break;
      
      case PendingOperationItem::AddAudioCtrlVal:
        if(poi._type == PendingOperationItem::AddAudioCtrlVal && poi._aud_ctrl_list == op._aud_ctrl_list)
        {
          // Simply replace the value.
          poi._ctl_dbl_val = op._ctl_dbl_val; 
          // An operation will still take place.
          return true;
        }
        else if(poi._type == PendingOperationItem::DeleteAudioCtrlVal && poi._aud_ctrl_list == op._aud_ctrl_list)
        {
          // Transform existing delete command into a modify command.
          poi._type = PendingOperationItem::ModifyAudioCtrlVal;
          poi._ctl_dbl_val = op._ctl_dbl_val; 
          // An operation will still take place.
          return true;
        }
        else if(poi._type == PendingOperationItem::ModifyAudioCtrlVal && poi._aud_ctrl_list == op._aud_ctrl_list)
        {
          // Simply replace the value.
          poi._ctl_dbl_val = op._ctl_dbl_val;
          // An operation will still take place.
          return true;
        }
      break;
      
      case PendingOperationItem::DeleteAudioCtrlVal:
        if(poi._type == PendingOperationItem::DeleteAudioCtrlVal && poi._aud_ctrl_list == op._aud_ctrl_list)
        {
          // Multiple delete commands not allowed! 
          ERROR_OPERATIONS(stderr, "MusE error: PendingOperationList::add(): Double DeleteAudioCtrlVal. Ignoring.\n");
          return false;
        }
        else if(poi._type == PendingOperationItem::AddAudioCtrlVal && poi._aud_ctrl_list == op._aud_ctrl_list)
        {
          // Add followed by delete is useless. Cancel out the add + delete by erasing the add command.
          erase(ipos->second);
          _map.erase(ipos);
          // No operation will take place.
          return false;
        }
        else if(poi._type == PendingOperationItem::ModifyAudioCtrlVal && poi._aud_ctrl_list == op._aud_ctrl_list)
        {
          // Modify followed by delete is equivalent to just deleting.
          // Transform existing modify command into a delete command.
          poi._type = PendingOperationItem::DeleteAudioCtrlVal;
          // An operation will still take place.
          return true;
        }
      break;
      
      case PendingOperationItem::ModifyAudioCtrlVal:
        if(poi._type == PendingOperationItem::ModifyAudioCtrlVal && poi._aud_ctrl_list == op._aud_ctrl_list)
        {
          // Simply replace the value.
          poi._ctl_dbl_val = op._ctl_dbl_val;
          // An operation will still take place.
          return true;
        }
        else if(poi._type == PendingOperationItem::DeleteAudioCtrlVal && poi._aud_ctrl_list == op._aud_ctrl_list)
        {
          // Transform existing delete command into a modify command.
          poi._type = PendingOperationItem::ModifyAudioCtrlVal;
          poi._ctl_dbl_val = op._ctl_dbl_val; 
          // An operation will still take place.
          return true;
        }
        else if(poi._type == PendingOperationItem::AddAudioCtrlVal && poi._aud_ctrl_list == op._aud_ctrl_list)
        {
          // Simply replace the add value with the modify value.
          poi._ctl_dbl_val = op._ctl_dbl_val; 
          // An operation will still take place.
          return true;
        }
      break;
      
      
      case PendingOperationItem::AddTempo:
        DEBUG_OPERATIONS(stderr, "PendingOperationList::add() AddTempo\n");
        if(poi._type == PendingOperationItem::AddTempo && poi._tempo_list == op._tempo_list)
        {
          ERROR_OPERATIONS(stderr, "MusE error: PendingOperationList::add(): Double AddTempo. Ignoring.\n");
          return false;  
          // Simply replace the value.
        }
        else if(poi._type == PendingOperationItem::DeleteTempo && poi._tempo_list == op._tempo_list)
        {
          // Delete followed by add. Cannot cancel them out because add already created a new object. Allow to add...
        }
        else if(poi._type == PendingOperationItem::ModifyTempo && poi._tempo_list == op._tempo_list)
        {
          // Modify followed by add. Error.
          ERROR_OPERATIONS(stderr, "MusE error: PendingOperationList::add(): ModifyTempo then AddTempo. Ignoring.\n");
          return false;  
        }
      break;
        
      case PendingOperationItem::DeleteTempo:
        DEBUG_OPERATIONS(stderr, "PendingOperationList::add() DeleteTempo\n");
        if(poi._type == PendingOperationItem::DeleteTempo && poi._tempo_list == op._tempo_list)
        {
          ERROR_OPERATIONS(stderr, "MusE error: PendingOperationList::add(): Double DeleteTempo. Ignoring.\n");
          return false;  
        }
        else if(poi._type == PendingOperationItem::AddTempo && poi._tempo_list == op._tempo_list)
        {
          // Add followed by delete. Cannot cancel them out because add already created a new object. Allow to delete...
        }
        else if(poi._type == PendingOperationItem::ModifyTempo && poi._tempo_list == op._tempo_list)
        {
          // Modify followed by delete is equivalent to just deleting.
          // Transform existing modify command into a delete command.
          poi._type = PendingOperationItem::DeleteTempo;
          // Modify's iterator will point one AFTER the delete iterator. So decrement the iterator.
          //--poi._iTEvent;
          // Replace the modify iterator with the delete iterator.
          poi._iTEvent = op._iTEvent;
          poi._tempo_event = op._tempo_event;
          // An operation will still take place.
          return true;
        }
      break;
      
      case PendingOperationItem::ModifyTempo:
        DEBUG_OPERATIONS(stderr, "PendingOperationList::add() ModifyTempo\n");
        if(poi._type == PendingOperationItem::ModifyTempo && poi._tempo_list == op._tempo_list)
        {
          // Simply replace the value.
          poi._intA = op._intA; 
          // An operation will still take place.
          return true;
        }
        else if(poi._type == PendingOperationItem::AddTempo && poi._tempo_list == op._tempo_list)
        {
          // Add followed by modify. Just replace the add value
          poi._tempo_event->tempo = op._iTEvent->second->tempo;
          // An operation will still take place.
          return true;
        }
        else if(poi._type == PendingOperationItem::DeleteTempo && poi._tempo_list == op._tempo_list)
        {
          // Transform existing delete command into a modify command.
          poi._type = PendingOperationItem::ModifyTempo;
          // Delete's iterator will point one BEFORE the modify iterator. So increment the iterator.
          //++poi._iTEvent;
          // Replace the delete iterator with the modify iterator.
          poi._iTEvent = op._iTEvent;
          // Grab the tempo.
          poi._intA = op._intA;
          // Delete always does normalize, so nowhere to grab this value from.
          //poi._intB = true;  
          // An operation will still take place.
          return true;
        }
      break;
      
      case PendingOperationItem::SetStaticTempo:
#ifdef _PENDING_OPS_DEBUG_
        fprintf(stderr, "PendingOperationList::add() SetStaticTempo\n");
#endif      
        if(poi._type == PendingOperationItem::SetStaticTempo && poi._tempo_list == op._tempo_list)
        {
          // Simply replace the value.
          poi._intA = op._intA; 
          // An operation will still take place.
          return true;
        }
      break;
        
      case PendingOperationItem::SetGlobalTempo:
        DEBUG_OPERATIONS(stderr, "PendingOperationList::add() SetGlobalTempo\n");
        if(poi._type == PendingOperationItem::SetGlobalTempo && poi._tempo_list == op._tempo_list)
        {
          // Simply replace the new value.
          poi._intA = op._intA; 
          // An operation will still take place.
          return true;
        }
      break;

      
      case PendingOperationItem::AddSig:
        DEBUG_OPERATIONS(stderr, "PendingOperationList::add() AddSig\n");
        if(poi._type == PendingOperationItem::AddSig && poi._sig_list == op._sig_list) 
        {
          ERROR_OPERATIONS(stderr, "MusE error: PendingOperationList::add(): Double AddSig. Ignoring.\n");
          return false;  
          // Simply replace the value.
        }
        else if(poi._type == PendingOperationItem::DeleteSig && poi._sig_list== op._sig_list) 
        {
          // Delete followed by add. Cannot cancel them out because add already created a new object. Allow to add...
        }
        else if(poi._type == PendingOperationItem::ModifySig && poi._sig_list == op._sig_list)
        {
          // Modify followed by add. Error.
          ERROR_OPERATIONS(stderr, "MusE error: PendingOperationList::add(): ModifySig then AddSig. Ignoring.\n");
          return false;  
        }
      break;
        
      case PendingOperationItem::DeleteSig:
        DEBUG_OPERATIONS(stderr, "PendingOperationList::add() DeleteSig\n");
        if(poi._type == PendingOperationItem::DeleteSig && poi._sig_list == op._sig_list)
        {
          ERROR_OPERATIONS(stderr, "MusE error: PendingOperationList::add(): Double DeleteSig. Ignoring.\n");
          return false;  
        }
        else if(poi._type == PendingOperationItem::AddSig && poi._sig_list == op._sig_list)
        {
          // Add followed by delete. Cannot cancel them out because add already created a new object. Allow to delete...
        }
        else if(poi._type == PendingOperationItem::ModifySig && poi._sig_list == op._sig_list)
        {
          // Modify followed by delete is equivalent to just deleting.
          // Transform existing modify command into a delete command.
          poi._type = PendingOperationItem::DeleteSig;
          // Modify's iterator will point one AFTER the delete iterator. So decrement the iterator.
          //--poi._iSigEvent;
          // Replace the modify iterator with the delete iterator.
          poi._iSigEvent = op._iSigEvent;
          poi._sig_event = op._sig_event;
          // An operation will still take place.
          return true;
        }
      break;
      
      case PendingOperationItem::ModifySig:
        DEBUG_OPERATIONS(stderr, "PendingOperationList::add() ModifySig\n");
        if(poi._type == PendingOperationItem::ModifySig && poi._sig_list == op._sig_list)
        {
          // Simply replace the value.
          poi._intA = op._intA; 
          poi._intB = op._intB; 
          // An operation will still take place.
          return true;
        }
        else if(poi._type == PendingOperationItem::AddSig && poi._sig_list == op._sig_list)
        {
          // Add followed by modify. Just replace the add value
          poi._sig_event->sig = op._iSigEvent->second->sig;
          // An operation will still take place.
          return true;
        }
        else if(poi._type == PendingOperationItem::DeleteSig && poi._sig_list == op._sig_list)
        {
          // Transform existing delete command into a modify command.
          poi._type = PendingOperationItem::ModifySig;
          // Delete's iterator will point one BEFORE the modify iterator. So increment the iterator.
          //++poi._iSigEvent;
          // Replace the delete iterator with the modify iterator.
          poi._iSigEvent = op._iSigEvent;
          // Grab the signature.
          poi._intA = op._intA;
          poi._intB = op._intB;
          // An operation will still take place.
          return true;
        }
      break;

      
      case PendingOperationItem::AddKey:
        DEBUG_OPERATIONS(stderr, "PendingOperationList::add() AddKey\n");
        if(poi._type == PendingOperationItem::AddKey && poi._key_list == op._key_list) 
        {
          // Simply replace the value.
          poi._intB = op._intB; 
          // An operation will still take place.
          return true;
        }
        else if(poi._type == PendingOperationItem::DeleteKey && poi._key_list== op._key_list) 
        {
          // Transform existing delete command into a modify command.
          poi._type = PendingOperationItem::ModifyKey;
          poi._intA = op._intB; 
          // An operation will still take place.
          return true;
        }
        else if(poi._type == PendingOperationItem::ModifyKey && poi._key_list == op._key_list)
        {
          // Simply replace the value.
          poi._intA = op._intB;
          // An operation will still take place.
          return true;
        }
      break;
        
      case PendingOperationItem::DeleteKey:
        DEBUG_OPERATIONS(stderr, "PendingOperationList::add() DeleteKey\n");
        if(poi._type == PendingOperationItem::DeleteKey && poi._key_list == op._key_list)
        {
          // Multiple delete commands not allowed! 
          ERROR_OPERATIONS(stderr, "MusE error: PendingOperationList::add(): Double DeleteKey. Ignoring.\n");
          return false;  
        }
        else if(poi._type == PendingOperationItem::AddKey && poi._key_list == op._key_list)
        {
          // Add followed by delete is useless. Cancel out the add + delete by erasing the add command.
          erase(ipos->second);
          _map.erase(ipos);
          // No operation will take place.
          return false;
        }
        else if(poi._type == PendingOperationItem::ModifyKey && poi._key_list == op._key_list)
        {
          // Modify followed by delete is equivalent to just deleting.
          // Transform existing modify command into a delete command.
          poi._type = PendingOperationItem::DeleteKey;
          // Modify's iterator will point one AFTER the delete iterator. So decrement the iterator.
          //--poi._iKeyEvent;
          // Replace the modify iterator with the delete iterator.
          poi._iKeyEvent = op._iKeyEvent;
          // An operation will still take place.
          return true;
        }
      break;
      
      case PendingOperationItem::ModifyKey:
        DEBUG_OPERATIONS(stderr, "PendingOperationList::add() ModifyKey\n");
        if(poi._type == PendingOperationItem::ModifyKey && poi._key_list == op._key_list)
        {
          // Simply replace the value.
          poi._intA = op._intA;
          // An operation will still take place.
          return true;
        }
        else if(poi._type == PendingOperationItem::AddKey && poi._key_list == op._key_list)
        {
          // Simply replace the add value with the modify value.
          poi._intB = op._intA; 
          // An operation will still take place.
          return true;
        }
        else if(poi._type == PendingOperationItem::DeleteKey && poi._key_list == op._key_list)
        {
          // Transform existing delete command into a modify command.
          poi._type = PendingOperationItem::ModifyKey;
          // Delete's iterator will point one BEFORE the modify iterator. So increment the iterator.
          //++poi._iKeyEvent;
          // Replace the delete iterator with the modify iterator.
          poi._iKeyEvent = op._iKeyEvent;
          // Replace the value.
          poi._intA = op._intA;
          // An operation will still take place.
          return true;
        }
      break;
      
      
      case PendingOperationItem::AddStretchListRatioAt:
        if(poi._type == PendingOperationItem::AddStretchListRatioAt && poi._stretch_list == op._stretch_list && 
           poi._stretch_type == op._stretch_type)
        {
          // Simply replace the value.
          poi._audio_converter_value = op._audio_converter_value; 
          return true;
        }
// Todo?
//         else if(poi._type == PendingOperationItem::DeleteStretchListRatioAt && poi._stretch_list == op._stretch_list && 
//            poi._stretch_type == op._stretch_type)
//         {
//           // Transform existing delete command into a modify command.
//           poi._type = PendingOperationItem::ModifyStretchListRatioAt;
//           poi._audio_converter_value = op._audio_converter_value; 
//           return true;
//         }
        else if(poi._type == PendingOperationItem::ModifyStretchListRatioAt && poi._stretch_list == op._stretch_list &&
           poi._stretch_type == op._stretch_type)
        {
          // Simply replace the value.
          poi._audio_converter_value = op._audio_converter_value;
          return true;
        }
      break;
      
      case PendingOperationItem::DeleteStretchListRatioAt:
        if(poi._type == PendingOperationItem::DeleteStretchListRatioAt && poi._stretch_list == op._stretch_list && 
           poi._stretch_type == op._stretch_type)
        {
          // Multiple delete commands not allowed! 
          ERROR_OPERATIONS(stderr, "MusE error: PendingOperationList::add(): Double DeleteStretchRatioAt. Ignoring.\n");
          return false;
        }
// Todo?
//         else if(poi._type == PendingOperationItem::AddStretchListRatioAt && poi._stretch_list == op._stretch_list)
//         {
//           // Add followed by delete is useless. Cancel out the add + delete by erasing the add command.
//           erase(ipos->second);
//           _map.erase(ipos);
//           return true;
//         }
//         else if(poi._type == PendingOperationItem::ModifyStretchListRatioAt && poi._stretch_list == op._stretch_list)
//         {
//           // Modify followed by delete is equivalent to just deleting.
//           // Transform existing modify command into a delete command.
//           poi._type = PendingOperationItem::DeleteStretchListRatioAt;
//           return true;
//         }
      break;
      
      case PendingOperationItem::ModifyStretchListRatioAt:
        if(poi._type == PendingOperationItem::ModifyStretchListRatioAt && poi._stretch_list == op._stretch_list &&
           poi._stretch_type == op._stretch_type)
        {
          // Simply replace the value.
          poi._audio_converter_value = op._audio_converter_value;
          return true;
        }
// Todo?
//         else if(poi._type == PendingOperationItem::DeleteStretchListRatioAt && poi._stretch_list == op._stretch_list && 
//            poi._stretch_type == op._stretch_type)
//         {
//           // Transform existing delete command into a modify command.
//           poi._type = PendingOperationItem::ModifyStretchListRatioAt;
//           poi._audio_converter_value = op._audio_converter_value; 
//           return true;
//         }
        else if(poi._type == PendingOperationItem::AddStretchListRatioAt && poi._stretch_list == op._stretch_list &&
           poi._stretch_type == op._stretch_type)
        {
          // Simply replace the add value with the modify value.
          poi._audio_converter_value = op._audio_converter_value; 
          return true;
        }
      break;


      case PendingOperationItem::ModifyStretchListRatio:
        if(poi._type == PendingOperationItem::ModifyStretchListRatio && poi._stretch_list == op._stretch_list &&
           poi._stretch_type == op._stretch_type)
        {
          // Simply replace the value.
          poi._audio_converter_value = op._audio_converter_value;
          return true;
        }
      break;


      case PendingOperationItem::ModifySongLength:
        DEBUG_OPERATIONS(stderr, "PendingOperationList::add() ModifySongLength\n");
        if(poi._type == PendingOperationItem::ModifySongLength)
        {
          // Simply replace the value.
          poi._intA = op._intA;
          // An operation will still take place.
          return true;
        }
      break;  

      case PendingOperationItem::EnableAllAudioControllers:
        DEBUG_OPERATIONS(stderr, "PendingOperationList::add() EnableAllAudioControllers\n");
        if(poi._type == PendingOperationItem::EnableAllAudioControllers)
        {
          ERROR_OPERATIONS(stderr, "MusE error: PendingOperationList::add(): Double EnableAllAudioControllers. Ignoring.\n");
          return false;  
        }
      break;  

      case PendingOperationItem::GlobalSelectAllEvents:
#ifdef _PENDING_OPS_DEBUG_
        fprintf(stderr, "PendingOperationList::add() GlobalSelectAllEvents\n");
#endif      
        if(poi._type == PendingOperationItem::GlobalSelectAllEvents && poi._track_list == op._track_list) 
        {
          if(poi._boolA == op._boolA)
          {
            fprintf(stderr, "MusE error: PendingOperationList::add(): Double GlobalSelectAllEvents. Ignoring.\n");
            return false;  
          }
          else
          {
            // Special: Do not 'cancel' out this one. The selecions may need to affect all events.
            // Simply replace the value.
            poi._boolA = op._boolA; 
            // An operation will still take place.
            return true;
          }
        }
      break;  

      case PendingOperationItem::ModifyAudioSamples:
// TODO Not quite right yet.
//         if(poi._type == PendingOperationItem::ModifyAudioSamples && 
//           // If attempting to repeatedly modify the same list, or, if progressively modifying (list to list to list etc).
//           poi._audioSamplesPointer && op._audioSamplesPointer &&
//           (*poi._audioSamplesPointer == *op._audioSamplesPointer || poi._newAudioSamples == op._newAudioSamples))
//         {
//           // Simply replace the list.
//           poi._newAudioSamples = op._newAudioSamples; 
//           poi._newAudioSamplesLen = op._newAudioSamplesLen; 
//           return true;
//         }
      break;
      
      case PendingOperationItem::SwitchMetronomeSettings:
        if(poi._type == PendingOperationItem::SwitchMetronomeSettings && 
          (poi._bool_pointer == op._bool_pointer))
        {
          if(poi._boolA == op._boolA)
          {
            fprintf(stderr, "MusE error: PendingOperationList::add(): Double SwitchMetronomeSettings. Ignoring.\n");
            // No operation will take place.
            return false;  
          }
          else
          {
            // Enable or disable followed by disable or enable is useless. Cancel out both by erasing the command.
            erase(ipos->second);
            _map.erase(ipos);
            // No operation will take place.
            return false;
          }
        }
      break;
      
      case PendingOperationItem::ModifyMetronomeAccentMap:
// TODO Not quite right yet.
//         if(poi._type == PendingOperationItem::ModifyMetronomeAccentMap && 
//           // If attempting to repeatedly modify the same list, or, if progressively modifying (list to list to list etc).
//           (poi._metroAccentsMap == op._metroAccentsMap || poi._newMetroAccentsMap == op._newMetroAccentsMap))
//         {
//           // Simply replace the list.
//           poi._newMetroAccentsMap = op._newMetroAccentsMap;
//           // An operation will still take place.
//           return true;
//         }
      break;
      
      case PendingOperationItem::SetExternalSyncFlag:
        if(poi._type == PendingOperationItem::SetExternalSyncFlag && 
          (poi._bool_pointer == op._bool_pointer))
        {
          if(poi._boolA == op._boolA)
          {
            fprintf(stderr, "MusE error: PendingOperationList::add(): Double SetExternalSyncFlag. Ignoring.\n");
            // No operation will take place.
            return false;  
          }
          else
          {
            // Enable or disable followed by disable or enable is useless. Cancel out both by erasing the command.
            erase(ipos->second);
            _map.erase(ipos);
            // No operation will take place.
            return false;
          }
        }
      break;
      
      case PendingOperationItem::SetUseJackTransport:
        if(poi._type == PendingOperationItem::SetUseJackTransport && 
          (poi._bool_pointer == op._bool_pointer))
        {
          if(poi._boolA == op._boolA)
          {
            fprintf(stderr, "MusE error: PendingOperationList::add(): Double SetUseJackTransport. Ignoring.\n");
            // No operation will take place.
            return false;  
          }
          else
          {
            // Enable or disable followed by disable or enable is useless. Cancel out both by erasing the command.
            erase(ipos->second);
            _map.erase(ipos);
            // No operation will take place.
            return false;
          }
        }
      break;
      
      case PendingOperationItem::SetUseMasterTrack:
        if(poi._type == PendingOperationItem::SetUseMasterTrack && 
          (poi._tempo_list == op._tempo_list))
        {
          if(poi._boolA == op._boolA)
          {
            fprintf(stderr, "MusE error: PendingOperationList::add(): Double SetUseMasterTrack. Ignoring.\n");
            // No operation will take place.
            return false;  
          }
          else
          {
            // Enable or disable followed by disable or enable is useless. Cancel out both by erasing the command.
            erase(ipos->second);
            _map.erase(ipos);
            // No operation will take place.
            return false;
          }
        }
      break;
      
      case PendingOperationItem::Uninitialized:
        ERROR_OPERATIONS(stderr, "MusE error: PendingOperationList::add(): Uninitialized item. Ignoring.\n");
        return false;  
      break;  
        
      default:
      break;  
    }
  }

  // Special for these types of lists: Because of the way these lists operate,
  //  a delete operation followed by a modify operation on the SAME iterator
  //  needs special attention: The delete operation will ERASE the iterator
  //  that the modify operation points to! Also the two operations will have 
  //  different sorting ticks and won't be caught in the above loop. 
  // The only way around it is to increment the modify iterator now so that it is 
  //  already pointing to the next item by the time the delete operation happens:
  if(op._type == PendingOperationItem::ModifyTempo || 
     op._type == PendingOperationItem::ModifySig || 
     op._type == PendingOperationItem::ModifyKey)
  {
    unsigned int idx = 0;
    if(op._type == PendingOperationItem::ModifyTempo)
      idx = op._iTEvent->first;
    else if(op._type == PendingOperationItem::ModifySig)
      idx = op._iSigEvent->first;
    else if(op._type == PendingOperationItem::ModifyKey)
      idx = op._iKeyEvent->first;
    
    iPendingOperationSortedRange r = _map.equal_range(idx);
    iPendingOperationSorted ipos = r.second;
    while(ipos != r.first)
    {
      --ipos;
      PendingOperationItem& poi = *ipos->second;
      
      if(op._type == PendingOperationItem::ModifyTempo)
      {
        if(poi._type == PendingOperationItem::DeleteTempo && poi._tempo_list == op._tempo_list)
        {
          DEBUG_OPERATIONS(stderr, "PendingOperationList::add() DeleteTempo + ModifyTempo: Incrementing modify iterator: idx:%u cur tempo:%d tick:%u\n", 
                  idx, op._iTEvent->second->tempo, op._iTEvent->second->tick);
          op._iTEvent++;
          break;
        }
      }
      else if(op._type == PendingOperationItem::ModifySig)
      {
        if(poi._type == PendingOperationItem::DeleteSig && poi._sig_list == op._sig_list)
        {
          DEBUG_OPERATIONS(stderr, "PendingOperationList::add() DeleteSig + ModifySig: Incrementing modify iterator: idx:%u cur sig:%d/%d tick:%u\n", 
                  idx, op._iSigEvent->second->sig.z, op._iSigEvent->second->sig.n, op._iSigEvent->second->tick);
          op._iSigEvent++;
          break;
        }
      }
      else if(op._type == PendingOperationItem::ModifyKey)
      {
        if(poi._type == PendingOperationItem::DeleteKey && poi._key_list == op._key_list)
        {
          DEBUG_OPERATIONS(stderr, "PendingOperationList::add() DeleteKey + ModifyKey: Incrementing modify iterator: idx:%u cur key:%d tick:%u\n", 
                  idx, op._iKeyEvent->second.key, op._iKeyEvent->second.tick);
          op._iKeyEvent++;
          break;
        }
      }
    }
  }
  
  iPendingOperation iipo = insert(end(), op);
  _map.insert(std::pair<unsigned int, iPendingOperation>(t, iipo));
  return true;
}

iPendingOperation PendingOperationList::findAllocationOp(const PendingOperationItem& op)
{
  iPendingOperationSortedRange r = _map.equal_range(op.getIndex());
  iPendingOperationSorted ipos = r.second;
  while(ipos != r.first)
  {
    --ipos;
    const PendingOperationItem& poi = *ipos->second;
    if(poi.isAllocationOp(op))  // Comparison.
      return ipos->second;
  }  
  return end();
}


//---------------------------------------------------------
//   addTimeSigOperation
//---------------------------------------------------------

bool PendingOperationList::addTimeSigOperation(unsigned tick, const MusECore::TimeSignature& s, MusECore::SigList* sl)
{
  //if (tick > MAX_TICK)
  //  tick = MAX_TICK;
  
  if (s.z == 0 || s.n == 0) {
        fprintf(stderr, "PendingOperationList::addOperation illegal time signature %d/%d\n", s.z, s.n);
        return false;
        }
  MusECore::iSigEvent e = sl->upper_bound(tick);
  if(tick == e->second->tick)
    add(PendingOperationItem(sl, e, s, MusECore::PendingOperationItem::ModifySig));
  else 
  {
    MusECore::PendingOperationItem poi(sl, 0, tick, PendingOperationItem::AddSig);
    MusECore::iPendingOperation ipo = findAllocationOp(poi);
    if(ipo != end())
    {
      MusECore::PendingOperationItem& poi = *ipo;
      // Simply replace the value.
      poi._sig_event->sig = s;
    }
    else
    {
      poi._sig_event = new MusECore::SigEvent(s, tick); // These are the desired tick and sig but...
      add(poi);                           //  add will do the proper swapping with next event.
    }
  }
  return true;
}

//---------------------------------------------------------
//   delTimeSigOperation
//---------------------------------------------------------

bool PendingOperationList::delTimeSigOperation(unsigned tick, MusECore::SigList* sl)
{
  MusECore::iSigEvent e = sl->find(tick);
  if (e == sl->end()) {
        printf("PendingOperationList::delTimeSigOperation tick:%d not found\n", tick);
        return false;
        }
  MusECore::PendingOperationItem poi(sl, e, PendingOperationItem::DeleteSig);
  add(poi);
  return true;
}

//---------------------------------------------------------
//   addTempoOperation
//---------------------------------------------------------

bool PendingOperationList::addTempoOperation(unsigned tick, int tempo, TempoList* tl)
{
  if (tick > MAX_TICK)
    tick = MAX_TICK;
  iTEvent e = tl->upper_bound(tick);

  if(tick == e->second->tick)
    add(PendingOperationItem(tl, e, tempo, PendingOperationItem::ModifyTempo));
  else 
  {
    PendingOperationItem poi(tl, 0, tick, PendingOperationItem::AddTempo);
    iPendingOperation ipo = findAllocationOp(poi);
    if(ipo != end())
    {
      PendingOperationItem& poi = *ipo;
      // Simply replace the value.
      poi._tempo_event->tempo = tempo;
    }
    else
    {
      poi._tempo_event = new TEvent(tempo, tick); // These are the desired tick and tempo but...
      add(poi);                               //  add will do the proper swapping with next event.
    }
  }
  return true;
}

//---------------------------------------------------------
//   delTempoOperation
//---------------------------------------------------------

bool PendingOperationList::delTempoOperation(unsigned tick, TempoList* tl)
{
  iTEvent e = tl->find(tick);
  if (e == tl->end()) {
        printf("PendingOperationList::delTempoOperation tick:%d not found\n", tick);
        return false;
        }
  PendingOperationItem poi(tl, e, PendingOperationItem::DeleteTempo);
  // NOTE: Deletion is done in post-RT stage 3.
  add(poi);
  return true;
}


} // namespace MusECore
  
