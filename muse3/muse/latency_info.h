//=========================================================
//  MusE
//  Linux Music Editor
//
//  latency_info.h
//  (C) Copyright 2019 Tim E. Real (terminator356 on users dot sourceforge dot net)
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

#ifndef __LATENCY_INFO_H__
#define __LATENCY_INFO_H__

namespace MusECore {

// During latency computations each process cycle,
//  this holds cached computed latency values.
struct TrackLatencyInfo
{
  // Whether the dominance information is valid (has already been
  //  gathered in the in the dominance latency scan).
  // This is reset near the beginning of the process handler.
  bool _dominanceProcessed;
  // Whether the 'forward' information is valid (has already been gathered
  //  in the forward latency scan).
//   // This is reset near the beginning of the process handler.
//   bool _forwardProcessed;
  // Whether the correction information is valid (has already been gathered
  //  in the correction latency scan).
  // This is reset near the beginning of the process handler.
  bool _correctionProcessed;
  // Whether the final latency information is valid (has already been gathered
  //  in the in the final latency scan).
  // This is reset near the beginning of the process handler.
  bool _processed;
  // Contributions to latency from rack plugins and/or Jack ports etc.
  // This value is the worst-case latency of all the channels in a track.
  // See AudioTrack::trackLatency().
  float _trackLatency;
//   float _forwardTrackLatency;
  // The absolute latency of all signals leaving a track, relative to audio driver frame (transport, etc).
  // This value is the cumulative value of all series routes connected to this track, plus some
  //  adjustment for the track's own members' latency.
  // The goal is to have equal latency output on all channels.
  // Thus the value will be the WORST-CASE latency of any channel. All other channels are delayed to match it.
  // For example, a Wave Track can use this total value to appropriately shift recordings of the signals
  //  arriving at its inputs.
  float _outputLatency;
//   float _forwardOutputLatency;
//   // Maximum amount of latency that this track's input can CORRECT (not just COMPENSATE).
//   float _inputAvailableCorrection;
//   float _forwardInputAvailableCorrection;
//   // Maximum amount of latency that this track's output can CORRECT (not just COMPENSATE).
//   float _outputAvailableCorrection;
//   float _forwardOutputAvailableCorrection;
  
  // Whether any of the connected output routes are effectively connected.
  // That means track is not off, track is monitored where applicable, etc,
  //   ie. signal can actually flow.
  bool _isLatencyInputTerminal;
  bool _isLatencyOutputTerminal;
  // Whether this track (and the branch it is in) can force other parallel branches to
  //  increase their latency compensation to match this one.
  // If false, this branch will NOT disturb other parallel branches' compensation,
  //  intead only allowing compensation UP TO the worst case in other branches.
  bool _canDominateOutputLatency;
//   bool _canDominateInputLatency;
  // Whether this track and its branch require latency correction, not just compensation.
  bool _requiresInputCorrection;
  // Whether this track and its branch can correct for latency, not just compensate.
  bool _canCorrectOutputLatency;
  // For tracks which can correct for latency, this is the value that the track
  //  must shift (ahead) to correct. It is essentially the programmed latency value
  //  of the track's ultimate source (wave file etc.). Therefore it will be NEGATIVE
  //  when requiring correction, but never POSITIVE (that would be unneccessary,
  //  a subsequent compensator delay can do that).
  float _sourceCorrectionValue;
  // Balances end points (Audio Outputs or open branches) of parallel branches.
  unsigned long int _compensatorWriteOffset;

  // Initializes (resets) the structure to prepare for (re)computation.
  void initialize();
};

} // namespace MusECore

#endif