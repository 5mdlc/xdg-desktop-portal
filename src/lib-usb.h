/*
 * Copyright © 2020 5mdlc
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors: 5mdlc
 */

//#ifndef LIB_USB_H
//#define LIB_USB_H
#pragma once

#include <gio/gio.h>

GDBusInterfaceSkeleton * lib_usb_create (GDBusConnection *connection);
//#endif//LIB_USB_H
