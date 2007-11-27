/********************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

Copyright (C) 2007 Lubos Lunak <l.lunak@kde.org>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*********************************************************************/

#include "fallapart.h"

#include <assert.h>
#include <math.h>

namespace KWin
{

KWIN_EFFECT( fallapart, FallApartEffect )

void FallApartEffect::prePaintScreen( ScreenPrePaintData& data, int time )
    {
    if( !windows.isEmpty())
        data.mask |= PAINT_SCREEN_WITH_TRANSFORMED_WINDOWS;
    effects->prePaintScreen(data, time);
    }

void FallApartEffect::prePaintWindow( EffectWindow* w, WindowPrePaintData& data, int time )
    {
    if( windows.contains( w ))
        {
        if( windows[ w ] < 1 )
            {
            windows[ w ] += time / 1000.;
            data.setTransformed();
            w->enablePainting( EffectWindow::PAINT_DISABLED_BY_DELETE );
            // Request the window to be divided into cells
            data.quads = data.quads.makeGrid( 40 );
            }
        else
            {
            windows.remove( w );
            w->unrefWindow();
            }
        }
    effects->prePaintWindow( w, data, time );
    }

void FallApartEffect::paintWindow( EffectWindow* w, int mask, QRegion region, WindowPaintData& data )
    {
    if( windows.contains( w ))
        {
        WindowQuadList new_quads;
        int cnt = 0;
        foreach( WindowQuad quad, data.quads )
            {
            // make fragments move in various directions, based on where
            // they are (left pieces generally move to the left, etc.)
            QPointF p1( quad[ 0 ].x(), quad[ 0 ].y());
            double xdiff = 0;
            if( p1.x() < w->width() / 2 )
                xdiff = -( w->width() / 2 - p1.x()) / w->width() * 100;
            if( p1.x() > w->width() / 2 )
                xdiff = ( p1.x() - w->width() / 2 ) / w->width() * 100;
            double ydiff = 0;
            if( p1.y() < w->height() / 2 )
                ydiff = -( w->height() / 2 - p1.y()) / w->height() * 100;
            if( p1.y() > w->height() / 2 )
                ydiff = ( p1.y() - w->height() / 2 ) / w->height() * 100;
            double modif = windows[ w ] * windows[ w ] * 64;
            srandom( cnt ); // change direction randomly but consistently
            xdiff += ( rand() % 21 - 10 );
            ydiff += ( rand() % 21 - 10 );
            for( int j = 0;
                 j < 4;
                 ++j )
                {
                quad[ j ].move( quad[ j ].x() + xdiff * modif, quad[ j ].y() + ydiff * modif );
                }
            // also make the fragments rotate around their center
            QPointF center(( quad[ 0 ].x() + quad[ 1 ].x() + quad[ 2 ].x() + quad[ 3 ].x()) / 4,
                ( quad[ 0 ].y() + quad[ 1 ].y() + quad[ 2 ].y() + quad[ 3 ].y()) / 4 );
            double adiff = ( rand() % 720 - 360 ) / 360. * 2 * M_PI; // spin randomly
            for( int j = 0;
                 j < 4;
                 ++j )
                {
                double x = quad[ j ].x() - center.x();
                double y = quad[ j ].y() - center.y();
                double angle = atan2( y, x );
                angle += windows[ w ] * adiff;
                double dist = sqrt( x * x + y * y );
                x = dist * cos( angle );
                y = dist * sin( angle );
                quad[ j ].move( center.x() + x, center.y() + y );
                }
            new_quads.append( quad );
            ++cnt;
            }
        data.quads = new_quads;
        }
    effects->paintWindow( w, mask, region, data );
    }

void FallApartEffect::postPaintScreen()
    {
    if( !windows.isEmpty())
        effects->addRepaintFull();
    effects->postPaintScreen();
    }

void FallApartEffect::windowClosed( EffectWindow* c )
    {
    windows[ c ] = 0;
    c->refWindow();
    }

void FallApartEffect::windowDeleted( EffectWindow* c )
    {
    windows.remove( c );
    }

} // namespace
