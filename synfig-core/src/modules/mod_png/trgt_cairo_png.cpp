/* === S Y N F I G ========================================================= */
/*!	\file trgt_png.cpp
**	\brief png_trgt Target Module
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007 Chris Moore
**
**	This package is free software; you can redistribute it and/or
**	modify it under the terms of the GNU General Public License as
**	published by the Free Software Foundation; either version 2 of
**	the License, or (at your option) any later version.
**
**	This package is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
**	General Public License for more details.
**	\endlegal
**
** === N O T E S ===========================================================
**
** ========================================================================= */

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "trgt_cairo_png.h"
#include <ETL/stringf>
#include <cstdio>
#include <algorithm>
#include <functional>
#include <ETL/misc>

#endif

/* === M A C R O S ========================================================= */

using namespace synfig;
using namespace std;
using namespace etl;

/* === G L O B A L S ======================================================= */

SYNFIG_TARGET_INIT(cairo_png_trgt);
SYNFIG_TARGET_SET_NAME(cairo_png_trgt,"cairo_png");
SYNFIG_TARGET_SET_EXT(cairo_png_trgt,"png");
SYNFIG_TARGET_SET_VERSION(cairo_png_trgt,"0.1");
SYNFIG_TARGET_SET_CVS_ID(cairo_png_trgt,"$Id$");

/* === M E T H O D S ======================================================= */

//Target *cairo_png_trgt::New(const char *filename){	return new cairo_png_trgt(filename);}

cairo_png_trgt::cairo_png_trgt(const char *Filename,
				   const synfig::TargetParam&  params )
{
	base_filename=Filename;
	filename=Filename;
	sequence_separator=params.sequence_separator;
}

cairo_png_trgt::~cairo_png_trgt()
{
}

bool
cairo_png_trgt::set_rend_desc(RendDesc *given_desc)
{
	//given_desc->set_pixel_format(PixelFormat((int)PF_RGB|(int)PF_A));
	desc=*given_desc;
	imagecount=desc.get_frame_start();
	if(desc.get_frame_end()-desc.get_frame_start()>0)
		multi_image=true;
	else
		multi_image=false;
	return true;
}

bool
cairo_png_trgt::obtain_surface(cairo_surface_t *&surface)
{
	if(filename=="-")
	{
		synfig::error("Cairo PNG surface does not support writing to stdout");
	}
	else if(multi_image)
	{
		filename = (filename_sans_extension(base_filename) +
						   sequence_separator +
						   etl::strprintf("%04d",imagecount) +
						   filename_extension(base_filename));
	}
	else
	{
		filename = base_filename;
	}

	int w=desc.get_w(), h=desc.get_h();
	surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, w, h);
	return true;
}

bool
cairo_png_trgt::put_surface(cairo_surface_t *surface, synfig::ProgressCallback *cb)
{
	gamma_filter(surface);
	if(cairo_surface_status(surface))
	{
		if(cb) cb->error(_("Cairo Surface bad status"));
		return false;
	}
	cairo_status_t status = cairo_surface_write_to_png(surface, filename.c_str());
	synfig::info(cairo_status_to_string(status));
	imagecount++;

	cairo_surface_destroy(surface);
	return true;
}

void
cairo_png_trgt::gamma_filter(cairo_surface_t *surface)
{
	CairoSurface temp(surface);
	temp.map_cairo_image();
	int x, y, w, h;
	float range(CairoColor::range);
	w=temp.get_w();
	h=temp.get_h();
	for(y=0;y<h; y++)
		for(x=0;x<w; x++)
		{
			CairoColor c(temp[y][x]);
			c=c.demult_alpha();
			c.set_r(gamma_in(c.get_r()/range)*range);
			c.set_g(gamma_in(c.get_g()/range)*range);
			c.set_b(gamma_in(c.get_b()/range)*range);
			c=c.premult_alpha();
			temp[y][x]=c;
		}
	temp.unmap_cairo_image();
}