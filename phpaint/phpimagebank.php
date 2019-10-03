<?
  if ($imtype=="gif") {Header( "Content-type: image/gif");}
  else if ($imtype=="wbmp") {Header( "Content-type: image/vnd.wap.wbmp");}
  else if ($imtype=="jpeg") {Header( "Content-type: image/jpeg");}
  else if ($imtype=="jpg") {Header( "Content-type: image/jpeg");}
  else if ($imtype=="printscript") {Header( "Content-type: text/plain");}
  else {Header( "Content-type: image/png");}
/* 
**********************************************************************
PHPaint v.2.1 for fast editing and previewing of images made on the fly 
using PHP, the GD Library, and FreeType. July 15, 2002.

Copyright 1999-2002 by Erica Andrews (PhrozenSmoke@yahoo.com).
http://phpaint.sourceforge.net

This program is free software. You can redistribute it and/or modify
it under the terms of the GNU General Public License (version 2)
as published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
**********************************************************************
 */ 
  if ($imtype=="printscript") {readfile( "phpimagebank.php");}
  else {
  if (! isset($fontdir)) {$fontdir="/var/www/html/ttf/";}
  $ffont=join("",array($fontdir,$font));
  $size = imagettfbbox($s,0,$ffont,$text);
  $dx = abs($size[2]-$size[0])+(int)($xpad*2.6);
  $dy = abs($size[5]-$size[3])+($ypad*2);
  $im = imagecreate($dx,$dy);
  $tshadingcolor=ImageColorAllocate($im,$im1,$im2,$im3); 
  $textshade = ImageColorAllocate($im,$ts1,$ts2,$ts3); 
  $imageshade = ImageColorAllocate($im,$is1,$is2,$is3); 
  $textcolor = ImageColorAllocate($im,$te1,$te2,$te3); 
  ImageRectangle($im,0,0,$dx,$dy-3,$imageshade);
  ImageRectangle($im,0,0,$dx-1,$dy,$tshadingcolor);
  imageline($im,1,$dy-2,$dx-1,$dy-2,$imageshade); 
  imageline($im,0,$dy-1,$dx-1,$dy-1,$imageshade); 
  imageline($im,0,$dy,$dx-1,$dy,$imageshade); 
  imageline($im,$dx-1,0,$dx-1,$dy-3,$imageshade); 
  // draw a highlight if possible
  $hi1=$im1+70;
  $hi2=$im2+70;
  $hi3=$im3+70;
  if ($hi1 > 255) {$hi1=255;}
  if ($hi2 > 255) {$hi2=255;}
  if ($hi3 > 255) {$hi3=255;}
  $highlight=ImageColorAllocate($im,$hi1,$hi2,$hi3);
  imageline($im,0,0,$dx-1,0,$highlight); 
  imageline($im,0,0,0,$dy-3,$highlight); 
  ImageTTFText($im, $s,0,(int)($xpad),$dy-(int)($ypad)+1,$textshade,$ffont,$text);
  ImageTTFText($im, $s,0,(int)($xpad)-1,$dy-(int)($ypad)-2,$textcolor,$ffont,$text);
  if ($imtype=="gif") {  ImageGif($im);}
  else if ($imtype=="wbmp") {  ImageWBMP($im);}
  else if ($imtype=="jpeg") {  ImageJpeg($im);}
  else if ($imtype=="jpg") {  ImageJpeg($im);}
  else {  ImagePng($im);}
  ImageDestroy($im); 
       }
?>