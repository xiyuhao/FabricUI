/*
 *  Copyright (c) 2010-2017 Fabric Software Inc. All rights reserved.
 */

#ifndef __FABRICUI_VIEWPORT_H__
#define __FABRICUI_VIEWPORT_H__

#include <FabricUI/Viewports/QtToKLEvent.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <algorithm>
#include <iterator>
#include <cmath>
#include <ostream>
#include <fstream>
#include <streambuf>
#include <memory>

#include <QtCore>
#include <QTime>
#include <QSettings>
#include <QMouseEvent>
#include <QImage>
#include <QGLWidget>

#include <FabricCore.h>

namespace FabricUI
{
  namespace Viewports
  {
    /// Defines in Canvas, not SceneHub
    class ManipulationTool;

    class ViewportWidget : public QGLWidget
    { 
      private:
        void init(
          FabricCore::Client &client, 
          QColor bgColor, 
          QSettings *settings = 0);

      public:
        /// Constructor for RTRGLViewportWidget.
        ViewportWidget(
          FabricCore::Client &client, 
          QColor bgColor, 
          QGLContext *qglContext, 
          QWidget *parent = 0, 
          QGLWidget *share = 0,
          QSettings *settings = 0);

        /// Constructor for GLViewportWidget.
      	ViewportWidget(
          FabricCore::Client &client, 
          QColor bgColor, 
          QGLFormat format, 
          QWidget *parent = 0, 
          QSettings *settings = 0);

      	virtual ~ViewportWidget() {};

        virtual double fps() { return m_fps; }
        QColor backgroundColor() { return m_bgColor; }
      
        FabricCore::Client getClient() { return m_client; }
        FabricCore::RTVal getViewport() { return m_viewport; }
        virtual FabricCore::RTVal getCamera() { FabricCore::RTVal val; return val; }
        virtual void setBackgroundColor(QColor color) {}
      
        // Canvas (InlineDrawing) specific
        virtual void clearInlineDrawing() {}
        virtual bool isManipulationActive() { return false; }
        virtual void setManipulationActive(bool state) {}
        virtual ManipulationTool *getManipTool() { return 0; }
        virtual bool isUsingStage() { return false; }
        virtual bool isStageVisible() { return false; }


      public slots:
        virtual void redraw() { update(); }
        virtual void onContextMenu(QPoint &point) {}
        

      protected:
        virtual void keyPressEvent(QKeyEvent * event);
        virtual void mousePressEvent(QMouseEvent *event);
        virtual void mouseMoveEvent(QMouseEvent *event);
        virtual void mouseReleaseEvent(QMouseEvent *event);
        virtual void wheelEvent(QWheelEvent *event);
        void computeFPS();
       

        double m_fps;
        bool m_hasCommercialLicense;
        double m_fpsStack[16];
        FabricCore::Client m_client;
     
        QColor m_bgColor;
        QTime m_fpsTimer;
        QSettings *m_settings;
        FabricCore::RTVal m_viewport;
    };
  }
}

#endif // __FABRICUI_VIEWPORT_H__
