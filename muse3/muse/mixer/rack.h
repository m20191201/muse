//=========================================================
//  MusE
//  Linux Music Editor
//  $Id: rack.h,v 1.5.2.3 2006/09/24 19:32:31 terminator356 Exp $
//
//  (C) Copyright 2000 Werner Schweer (ws@seh.de)
//  (C) Copyright 2016 Tim E. Real (terminator356 on sourceforge)
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

#ifndef __RACK_H__
#define __RACK_H__

#include <QListWidget>
#include "type_defs.h"

#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QDropEvent>
#include <QMouseEvent>
#include <QEvent>
#include <QColor>

#include "track.h"
#include "xml.h"
#include "background_painter.h"

namespace MusEGui {

//---------------------------------------------------------
//   EffectRack
//---------------------------------------------------------

class EffectRack : public QListWidget {
      Q_OBJECT
    
      Q_PROPERTY( QColor activeColor READ activeColor WRITE setActiveColor )
    
      MusECore::AudioTrack* track;
      int itemheight;
      QColor _activeColor;
      ItemBackgroundPainter* _bkgPainter;

      virtual QSize minimumSizeHint() const;
      virtual QSize sizeHint() const;
      
      void startDragItem(int idx);
      void initPlugin(MusECore::Xml xml, int idx);
      QPoint dragPos;
      void savePreset(int idx);
      void choosePlugin(QListWidgetItem* item, bool replace = false);

   private slots:
      void menuRequested(QListWidgetItem*);
      void doubleClicked(QListWidgetItem*);
      void songChanged(MusECore::SongChangedStruct_t);
      void updateContents();

   protected:
      void dropEvent(QDropEvent *event);
      void dragEnterEvent(QDragEnterEvent *event);
      void mousePressEvent(QMouseEvent *event);
      void mouseMoveEvent(QMouseEvent *event);
      void enterEvent(QEvent *event);
      void leaveEvent(QEvent *event);

      QStringList mimeTypes() const;
      Qt::DropActions supportedDropActions () const;
   
   public:
      EffectRack(QWidget*, MusECore::AudioTrack* t);

      MusECore::AudioTrack* getTrack() const { return track; } 
      QPoint getDragPos() const { return dragPos; }
      QColor activeColor() const { return _activeColor; }
      void setActiveColor(const QColor& c) { _activeColor = c; update(); }
      ItemBackgroundPainter* getBkgPainter() const { return _bkgPainter; }

      };

} // namespace MusEGui

#endif

