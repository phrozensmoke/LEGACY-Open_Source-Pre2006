<?php
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

if (!isset($xpad)) { $xpad = 6; } 
if (!isset($ypad)) { $ypad = 6 ;} 
if (!isset($backgroundcolor)) { $backgroundcolor = "#FFFFFF" ;} 
if (!isset($tshadingcolor)) { $tshadingcolor = "60,60,60" ;} 
if (!isset($imagecolor)) { $imagecolor = "30,200,200" ;} 
if (!isset($textcolor)) { $textcolor = "0,255,255" ;} 
if (!isset($ishadingcolor)) { $ishadingcolor = "0,160,160" ;} 
if (! isset($font)) {if (isset($fontalt)) { $font = $fontalt; } else { $font = 
"shatter.ttf" ;}
  }
if (!isset($fontalt)) { $comment = "<font size=-1>Sample image using the 
shatter.ttf font</font>" ;} 
else {$comment =" " ; }
if (!isset($s)) { $s =40 ;}
if (!isset($text)) { $text ="SAMPLE IMAGE" ;} 
$tshad = explode("," , $tshadingcolor); 
$ts1 = $tshad[0]; 
$ts2 = $tshad[1]; 
$ts3 = $tshad[2]; 
$imag = explode("," , $imagecolor); 
$im1 = $imag[0]; 
$im2 = $imag[1]; 
$im3 = $imag[2]; 
$textc = explode("," , $textcolor); 
$te1 = $textc[0]; 
$te2 = $textc[1]; 
$te3 = $textc[2]; 
$ishad = explode("," , $ishadingcolor); 
$is1 = $ishad[0]; 
$is2 = $ishad[1]; 
$is3 = $ishad[2]; 
?>
<html><title>PHPaint v.2.1 by Erica Andrews</title> 
<head>
             
<script language="javascript"> 
function ChangeIshade(rgb) { document.form1.ishadingcolor.value=rgb;}
function ChangeBG(rgb) { document.form1.backgroundcolor.value=rgb; document.form1.submit();}
function ChangeIMColor(rgb) { document.form1.imagecolor.value=rgb;}
function ChangeTextColor(rgb) { document.form1.textcolor.value=rgb;}
function ChangeTshade(rgb) { document.form1.tshadingcolor.value=rgb;}
function dofont() { document.form1.fontalt.value = document.form1.font.value ; 
                  }
</script>


</head>
<body topmargin="0" leftmargin="0" vlink="#0000FF" bgcolor="#EEEEEE">

<form name="form1" action="<?php echo $PHP_SELF; ?>" method="post">
<table BORDER=0 WIDTH="800" >
<tr>
<td ALIGN=CENTER VALIGN=TOP WIDTH="232">


<center><table BORDER=1  WIDTH="237" BGCOLOR="#CCCCCC" >
<tr>
<td ALIGN=CENTER VALIGN=TOP><b><font face="Times"><font 
color="#000000">Text Shading
Color</font></font></b>
</center><center><input type=text size="11" length="11" maxlength="11" 
name="tshadingcolor" value="<?php echo $tshadingcolor; 
?>"></center><center><IMG SRC="palette.gif" USEMAP="#palette.gif" WIDTH=228 
HEIGHT=36 BORDER=0>
<MAP NAME="palette.gif">
<AREA SHAPE=RECT COORDS="2,1,16,17" ALT="0,0,0" href="javascript:ChangeTshade('0,0,0');"
OnMouseOver="window.status='0,0,0'; return true"
OnMouseOut="window.status='Color Palette'; return true"><AREA SHAPE=RECT
COORDS="17,0,32,16" ALT="128,128,128" href="javascript:ChangeTshade('128,128,128')"
OnMouseOver="window.status='128,128,128'; return true"
OnMouseOut="window.status='Color Palette'; return true"><AREA SHAPE=RECT
COORDS="32,0,49,16" ALT="128,0,64"  href="javascript:ChangeTshade('128,0,64')"
OnMouseOver="window.status='128,0,64'; return true"><AREA SHAPE=RECT
COORDS="49,1,64,15" ALT="128,128,0"  href="javascript:ChangeTshade('128,128,0')"
OnMouseOver="window.status='128,128,0'; return true">
<AREA SHAPE=RECT COORDS="67,0,80,17" ALT="0,128,64" href="javascript:ChangeTshade('0,128,64')"
OnMouseOver="window.status='0,128,64'; return true"><AREA SHAPE=RECT
COORDS="80,1,96,18" ALT="0,128,128" href="javascript:ChangeTshade('0,128,128')"
OnMouseOver="window.status='0,128,128'; return true"><AREA SHAPE=RECT
COORDS="96,1,112,16" ALT="0,0,128"  href="javascript:ChangeTshade('0,0,128')"
OnMouseOver="window.status='0,0,128'; return true"><AREA SHAPE=RECT 
COORDS="112,1,127,16" ALT="128,0,128" href="javascript:ChangeTshade('128,0,128')"
OnMouseOver="window.status='128,0,128'; return true">
<AREA SHAPE=RECT COORDS="129,0,144,17" ALT="128,128,64" href="javascript:ChangeTshade('128,128,64')"
OnMouseOver="window.status='128,128,64'; return true"><AREA SHAPE=RECT
COORDS="145,0,161,16" ALT="0,64,64"   href="javascript:ChangeTshade('0,64,64')"
OnMouseOver="window.status='0,64,64'; return true"><AREA SHAPE=RECT
COORDS="161,1,176,16" ALT="0,128,255"   href="javascript:ChangeTshade('0,128,255')"
OnMouseOver="window.status='0,128,255'; return true"><AREA SHAPE=RECT
COORDS="178,2,192,16" ALT="0,64,128"    href="javascript:ChangeTshade('0,64,128')"
OnMouseOver="window.status='0,64,128'; return true">
<AREA SHAPE=RECT COORDS="195,2,209,15" ALT="64,0,255"      href="javascript:ChangeTshade('64,0,255')"
OnMouseOver="window.status='64,0,255'; return true"><AREA SHAPE=RECT
COORDS="211,2,225,15" ALT="128,64,0"        href="javascript:ChangeTshade('128,64,0')"
OnMouseOver="window.status='128,64,0'; return true"><AREA SHAPE=RECT
COORDS="1,15,18,33" ALT="255,255,255"  href="javascript:ChangeTshade('255,255,255')"
OnMouseOver="window.status='255,255,255'; return true">
<AREA SHAPE=RECT COORDS="18,16,34,33" ALT="192,192,192"  href="javascript:ChangeTshade('192,192,192')"
OnMouseOver="window.status='192,192,192'; return true"><AREA SHAPE=RECT
COORDS="35,16,50,33" ALT="255,0,0"    href="javascript:ChangeTshade('255,0,0')"
OnMouseOver="window.status='255,0,0'; return true"><AREA SHAPE=RECT
COORDS="51,16,65,33" ALT="255,255,0"    href="javascript:ChangeTshade('255,255,0')"
OnMouseOver="window.status='255,255,0'; return true"><AREA SHAPE=RECT
COORDS="67,16,82,32" ALT="0,255,0"    href="javascript:ChangeTshade('0,255,0')"
OnMouseOver="window.status='0,255,0'; return true">
<AREA SHAPE=RECT COORDS="83,17,97,34" ALT="0,255,255"   href="javascript:ChangeTshade('0,255,255')"
OnMouseOver="window.status='0,255,255'; return true"><AREA SHAPE=RECT 
COORDS="98,16,112,33" ALT="0,0,255"   href="javascript:ChangeTshade('0,0,255')"
OnMouseOver="window.status='0,0,255'; return true"><AREA SHAPE=RECT
COORDS="114,16,130,33" ALT="255,0,255"   href="javascript:ChangeTshade('255,0,255')"
OnMouseOver="window.status='255,0,255'; return true">
<AREA SHAPE=RECT COORDS="133,17,145,35" ALT="255,255,128"
  href="javascript:ChangeTshade('255,255,128')"  OnMouseOver="window.status='255,255,128'; return
true"><AREA SHAPE=RECT COORDS="148,16,162,33" ALT="128,255,128"
  href="javascript:ChangeTshade('128,255,128')"    OnMouseOver="window.status='128,255,128'; return
true"><AREA SHAPE=RECT COORDS="162,17,178,34" ALT="128,255,255"
  href="javascript:ChangeTshade('128,255,255')"     OnMouseOver="window.status='128,255,255'; return true">
<AREA SHAPE=RECT COORDS="179,16,192,35" ALT="128,128,255" 
   href="javascript:ChangeTshade('128,128,255')"    OnMouseOver="window.status='128,128,255'; return
true"><AREA SHAPE=RECT COORDS="196,16,210,32" ALT="255,0,128" 
   href="javascript:ChangeTshade('255,0,128')"    OnMouseOver="window.status='255,0,128'; return
true"><AREA SHAPE=RECT COORDS="210,16,224,32" ALT="255,128,64" 
   href="javascript:ChangeTshade('255,128,64')"    OnMouseOver="window.status='255,128,64'; return true">
</MAP>
</center><center></font>
</td>
</tr>
</table></center>

<center><table BORDER=1 COLS=1 WIDTH="237" BGCOLOR="#C0C0C0" >
<tr>
<td>
<center><b><font face="Times"><font color="#000000">Background
Preview</font></font></b></center><center><input type=hidden size="11" 
length="11" maxlength="11" name="backgroundcolor" value="<?php echo 
$backgroundcolor; ?>"></center><center><font size=-2 face="Times" 
color="#000000"><IMG SRC="palette.gif" USEMAP="#palette2.gif" WIDTH=228 
HEIGHT=36 BORDER=0>
<MAP NAME="palette2.gif">
<AREA SHAPE=RECT COORDS="2,1,16,17" ALT="0,0,0" href="javascript:ChangeBG('#000000');"
OnMouseOver="window.status='0,0,0'; return true"
OnMouseOut="window.status='Color Palette'; return true"><AREA SHAPE=RECT
COORDS="17,0,32,16" ALT="128,128,128" href="javascript:ChangeBG('#808080')"
OnMouseOver="window.status='128,128,128'; return true"
OnMouseOut="window.status='Color Palette'; return true"><AREA SHAPE=RECT
COORDS="32,0,49,16" ALT="128,0,64"  href="javascript:ChangeBG('#800000')"
OnMouseOver="window.status='128,0,64'; return true"><AREA SHAPE=RECT
COORDS="49,1,64,15" ALT="128,128,0"  href="javascript:ChangeBG('#808000')"
OnMouseOver="window.status='128,128,0'; return true">
<AREA SHAPE=RECT COORDS="67,0,80,17" ALT="0,128,64" href="javascript:ChangeBG('#008000')"
OnMouseOver="window.status='0,128,64'; return true"><AREA SHAPE=RECT
COORDS="80,1,96,18" ALT="0,128,128" href="javascript:ChangeBG('#008080')"
OnMouseOver="window.status='0,128,128'; return true"><AREA SHAPE=RECT
COORDS="96,1,112,16" ALT="0,0,128"  href="javascript:ChangeBG('#000080')"
OnMouseOver="window.status='0,0,128'; return true"><AREA SHAPE=RECT 
COORDS="112,1,127,16" ALT="128,0,128" href="javascript:ChangeBG('#800080')"
OnMouseOver="window.status='128,0,128'; return true">
<AREA SHAPE=RECT COORDS="129,0,144,17" ALT="128,128,64" href="javascript:ChangeBG('#808040')"
OnMouseOver="window.status='128,128,64'; return true"><AREA SHAPE=RECT
COORDS="145,0,161,16" ALT="0,64,64"   href="javascript:ChangeBG('#004040')"
OnMouseOver="window.status='0,64,64'; return true"><AREA SHAPE=RECT
COORDS="161,1,176,16" ALT="0,128,255"   href="javascript:ChangeBG('#0080FF')"
OnMouseOver="window.status='0,128,255'; return true"><AREA SHAPE=RECT
COORDS="178,2,192,16" ALT="0,64,128"    href="javascript:ChangeBG('#004080')"
OnMouseOver="window.status='0,64,128'; return true">
<AREA SHAPE=RECT COORDS="195,2,209,15" ALT="64,0,255"      href="javascript:ChangeBG('#4000FF')"
OnMouseOver="window.status='64,0,255'; return true"><AREA SHAPE=RECT
COORDS="211,2,225,15" ALT="128,64,0"        href="javascript:ChangeBG('#804000')"
OnMouseOver="window.status='128,64,0'; return true"><AREA SHAPE=RECT
COORDS="1,15,18,33" ALT="255,255,255"  href="javascript:ChangeBG('#FFFFFF')"
OnMouseOver="window.status='255,255,255'; return true">
<AREA SHAPE=RECT COORDS="18,16,34,33" ALT="192,192,192"  href="javascript:ChangeBG('#C0C0C0')"
OnMouseOver="window.status='192,192,192'; return true"><AREA SHAPE=RECT
COORDS="35,16,50,33" ALT="255,0,0"    href="javascript:ChangeBG('#FF0000')"
OnMouseOver="window.status='255,0,0'; return true"><AREA SHAPE=RECT
COORDS="51,16,65,33" ALT="255,255,0"    href="javascript:ChangeBG('#FFFF00')"
OnMouseOver="window.status='255,255,0'; return true"><AREA SHAPE=RECT
COORDS="67,16,82,32" ALT="0,255,0"    href="javascript:ChangeBG('#00FF00')"
OnMouseOver="window.status='0,255,0'; return true">
<AREA SHAPE=RECT COORDS="83,17,97,34" ALT="0,255,255"   href="javascript:ChangeBG('#00FFFF')"
OnMouseOver="window.status='0,255,255'; return true"><AREA SHAPE=RECT 
COORDS="98,16,112,33" ALT="0,0,255"   href="javascript:ChangeBG('#0000FF')"
OnMouseOver="window.status='0,0,255'; return true"><AREA SHAPE=RECT
COORDS="114,16,130,33" ALT="255,0,255"   href="javascript:ChangeBG('#FF00FF')"
OnMouseOver="window.status='255,0,255'; return true">
<AREA SHAPE=RECT COORDS="133,17,145,35" ALT="255,255,128"
  href="javascript:ChangeBG('#FFFF80')"  OnMouseOver="window.status='255,255,128'; return
true"><AREA SHAPE=RECT COORDS="148,16,162,33" ALT="128,255,128"
  href="javascript:ChangeBG('#00FF80')"    OnMouseOver="window.status='128,255,128'; return
true"><AREA SHAPE=RECT COORDS="162,17,178,34" ALT="128,255,255"
  href="javascript:ChangeBG('#80FFFF')"     OnMouseOver="window.status='128,255,255'; return true">
<AREA SHAPE=RECT COORDS="179,16,192,35" ALT="128,128,255" 
   href="javascript:ChangeBG('#8080FF')"    OnMouseOver="window.status='128,128,255'; return
true"><AREA SHAPE=RECT COORDS="196,16,210,32" ALT="255,0,128" 
   href="javascript:ChangeBG('#FF0080')"    OnMouseOver="window.status='255,0,128'; return
true"><AREA SHAPE=RECT COORDS="210,16,224,32" ALT="255,128,64" 
   href="javascript:ChangeBG('#FF8040')"    OnMouseOver="window.status='255,128,64'; return true">
</MAP>

Change the background behind the image to see what it will look like on your 
web page.</center><center></font>

</center>
</td>
</tr>
</table></center>

<center><table BORDER=1 WIDTH="237" BGCOLOR="#CCCCCC" >
<tr>
<td ALIGN=CENTER VALIGN=TOP><b><font face="Times"><font 
color="#000000">Spacing Around Text</font></font></b>
</center><center>X-pad: <input type=text size="2" length="2" maxlength="2" 
name="xpad" value="<?php echo $xpad; ?>"> &nbsp;&nbsp; &nbsp;&nbsp;Y-pad: <input type=text size="2" 
length="2" maxlength="2" name="ypad" value="<?php echo $ypad; ?>"> 
</center><center><font size=-2 face="Times" color="#000000">Enter 
the number of pixels.</center><center></font></font>
</td>
</tr>
</table></center>

<center><table BORDER=1 WIDTH="237" BGCOLOR="#CCCCCC" >
<tr>
<td ALIGN=CENTER VALIGN=TOP> 
<font size=3 color="#000000" face="Times">
<b>Image Type:</b><br>
<select name="imtype">
<?php 
 if (isset($imtype)) { 
  if (trim($imtype) != "") {echo "<option value='$imtype'>$imtype</option>";}
                     }
 if ($imtype != "png") { echo "<option value='png'>png</option>"; }
 if ($imtype != "jpeg") { echo "<option value='jpeg'>jpeg</option>"; }
 if ($imtype != "wbmp") { echo "<option value='wbmp'>wbmp</option>"; }
 if ($imtype != "gif") { echo "<option value='gif'>gif</option>"; }
 if (! isset($imtype)) {$imtype="png";}
 if (!isset($fontdir)) {$fontdir="" .getcwd()."/";}
?>
</select>
<br>
</td>
</tr>
</table></center>


<table BORDER=1 WIDTH="237" BGCOLOR="#CCCCCC" >
<tr>
<td ALIGN=LEFT VALIGN=TOP> 
<font size=3 color="#000000" face="Times">
<table BGCOLOR="#CCCCCC" border=0>

<tr><td align=left><b>Link: </b>&nbsp;</td><td align=right><input type=text name=link value="<? echo $link;?>"><br></td></tr>
<tr><td align=left><b>Target: </b>&nbsp;</td><td align=right><input type=text name=target value="<? echo $target;?>"><br></td></tr>

</table>
</td>
</tr>
</table>

<!-- end left panel -->
</td>

<td ALIGN=CENTER VALIGN=TOP>
<table name="tableback" BORDER=1  COLS=1 WIDTH="350" HEIGHT="100%" >
<tr>
<td ALIGN=LEFT VALIGN=top BGCOLOR="<?php echo $backgroundcolor; ?>">  
<input type=hidden name="fontalt" value="<?php echo $font; ?>">
<br><center><font size=-2 color="#000000" face="Times">
<img alt="No TTF Font Was Selected, incorrect Font Directory, or Image Type not supported" src="phpimagebank.php?s=<?php echo $s; 
?>&text=<?php echo ereg_replace(" ","+",$text); ?>&ts1=<?php echo $ts1; 
?>&ts2=<?php echo $ts2; ?>&ts3=<?php echo $ts3; ?>&im1=<?php echo $im1; 
?>&im2=<?php echo $im2; ?>&im3=<?php echo $im3; ?>&te1=<?php echo $te1; 
?>&te2=<?php echo $te2; ?>&te3=<?php echo $te3; ?>&is1=<?php echo $is1; 
?>&is2=<?php echo $is2; ?>&is3=<?php echo $is3; ?>&font=<?php echo $font; 
?>&xpad=<?php echo $xpad; ?>&ypad=<?php echo $ypad; ?>&fontdir=<?php echo $fontdir; ?>&imtype=<?php echo $imtype; ?>">
<br><?php echo $comment; ?><br><br><input type=submit value="CREATE 
IMAGE"></center><br></font></td></tr><td ALIGN=LEFT VALIGN=top>



<font size=-1 color="#000000" face="Times">
<b>TrueType Font:</b>  

<?php
echo "<form><select name='font' onclick='dofont()'>";
    $handle=opendir($fontdir);
    while ($file = readdir($handle)) { 
      if  ($font== $file) {echo "<option value='$file' 
selected>$file</option>\n";} 
                                     }
    closedir($handle); 

    $handle=opendir($fontdir);
    while ($file = readdir($handle)) {

 if (strpos(strtolower($file),".ttf")) {echo "<option
value='$file'>$file</option>\n";}
 
                                     }
    closedir($handle);


?>
</select>

<input type=hidden name='fontdir' value='<?php echo $fontdir;
?>'>&nbsp;&nbsp;
<script language='javascript'>
function promptDir() {
   r=prompt("Please enter directory to look for fonts. Include trailing slash:",""+document.form1.fontdir.value);
   if (r==null) {return;}
   document.form1.fontdir.value=r
   document.form1.submit()
                     }
</script>
&nbsp; &nbsp;<input type=Button value='Font Directory...' onclick='javascript:promptDir();'>

<br><br><b>Font Size:</b> <input type="text" name="s" maxlength="3" 
size="3" value="<?php echo $s; ?>"><br><br>
 <b>Text On Image:</b>&nbsp;<input type="text" name="text" value="<?php echo 
$text; ?>"><br>

<?php 
$addon1="";
$addon2="";
if (isset($link)) {
 if (trim($link) != "") {
  $addon1="&lt;a href=\"$link\"";
  if (isset($target)) {
    if (trim($target) != "") { $addon1=$addon1." target=\"$target\""; }
                      }
  $addon1=$addon1.">";
  $addon2="&lt;/a>";
                        }
                  }
?>

 <br><br><b>IMG Tag For This Image:</b><br>
<textarea cols="34" rows="5" wrap=TRUE>
<? echo $addon1;?>&lt;img src="phpimagebank.php?s=<?php echo $s; ?>&amp;text=<?php echo
ereg_replace(" ","+",$text); ?>&amp;ts1=<?php echo $ts1; ?>&amp;ts2=<?php echo 
$ts2; ?>&amp;ts3=<?php echo $ts3; ?>&amp;im1=<?php echo $im1; ?>&amp;im2=<?php 
echo $im2; ?>&amp;im3=<?php echo $im3; ?>&amp;te1=<?php echo $te1; 
?>&amp;te2=<?php echo $te2; ?>&amp;te3=<?php echo $te3; ?>&amp;is1=<?php echo 
$is1; ?>&amp;is2=<?php echo $is2; ?>&amp;is3=<?php echo $is3; 
?>&amp;font=<?php echo $font; ?>&amp;xpad=<?php echo $xpad; ?>&amp;ypad=<?php 
echo $ypad; ?>&amp;fontdir=<?php echo $fontdir;?>" border=0 alt='<?php echo 
$text; ?>'><? echo $addon2;?>
</textarea><br> 

<br>
To use this image in your page, copy and paste the IMG tag (above) into your page. Then put the <a 
href="phpimagebank.php?imtype=printscript">Re-usable PHP script</a> in the same directory as your web page. Leave your <u><?php echo $font; ?></u> font in the '<?php echo $fontdir; ?>' directory. There is NO need to save a copy of this image unless you just want to.

</td>
</tr>
</table></center>
</td>

<td WIDTH="232" valign=top>
<center><table BORDER=1 WIDTH="237" BGCOLOR="#CCCCCC" >
<tr>
<td ALIGN=CENTER VALIGN=TOP><b><font face="Times"><font 
color="#000000">Main Image
Color</font></font></b>

</center><center><input type=text size="11" length="11" maxlength="11" 
name="imagecolor" value="<?php echo $imagecolor; ?>"></center><center><font 
size=-2 face="Times" color="#000000"><IMG SRC="palette.gif" 
USEMAP="#palette3.gif" WIDTH=228 HEIGHT=36 BORDER=0>
<MAP NAME="palette3.gif">
<AREA SHAPE=RECT COORDS="2,1,16,17" ALT="0,0,0" href="javascript:ChangeIMColor('0,0,0');"
OnMouseOver="window.status='0,0,0'; return true"
OnMouseOut="window.status='Color Palette'; return true"><AREA SHAPE=RECT
COORDS="17,0,32,16" ALT="128,128,128" href="javascript:ChangeIMColor('128,128,128')"
OnMouseOver="window.status='128,128,128'; return true"
OnMouseOut="window.status='Color Palette'; return true"><AREA SHAPE=RECT
COORDS="32,0,49,16" ALT="128,0,64"  href="javascript:ChangeIMColor('128,0,64')"
OnMouseOver="window.status='128,0,64'; return true"><AREA SHAPE=RECT
COORDS="49,1,64,15" ALT="128,128,0"  href="javascript:ChangeIMColor('128,128,0')"
OnMouseOver="window.status='128,128,0'; return true">
<AREA SHAPE=RECT COORDS="67,0,80,17" ALT="0,128,64" href="javascript:ChangeIMColor('0,128,64')"
OnMouseOver="window.status='0,128,64'; return true"><AREA SHAPE=RECT
COORDS="80,1,96,18" ALT="0,128,128" href="javascript:ChangeIMColor('0,128,128')"
OnMouseOver="window.status='0,128,128'; return true"><AREA SHAPE=RECT
COORDS="96,1,112,16" ALT="0,0,128"  href="javascript:ChangeIMColor('0,0,128')"
OnMouseOver="window.status='0,0,128'; return true"><AREA SHAPE=RECT 
COORDS="112,1,127,16" ALT="128,0,128" href="javascript:ChangeIMColor('128,0,128')"
OnMouseOver="window.status='128,0,128'; return true">
<AREA SHAPE=RECT COORDS="129,0,144,17" ALT="128,128,64" href="javascript:ChangeIMColor('128,128,64')"
OnMouseOver="window.status='128,128,64'; return true"><AREA SHAPE=RECT
COORDS="145,0,161,16" ALT="0,64,64"   href="javascript:ChangeIMColor('0,64,64')"
OnMouseOver="window.status='0,64,64'; return true"><AREA SHAPE=RECT
COORDS="161,1,176,16" ALT="0,128,255"   href="javascript:ChangeIMColor('0,128,255')"
OnMouseOver="window.status='0,128,255'; return true"><AREA SHAPE=RECT
COORDS="178,2,192,16" ALT="0,64,128"    href="javascript:ChangeIMColor('0,64,128')"
OnMouseOver="window.status='0,64,128'; return true">
<AREA SHAPE=RECT COORDS="195,2,209,15" ALT="64,0,255"      href="javascript:ChangeIMColor('64,0,255')"
OnMouseOver="window.status='64,0,255'; return true"><AREA SHAPE=RECT
COORDS="211,2,225,15" ALT="128,64,0"        href="javascript:ChangeIMColor('128,64,0')"
OnMouseOver="window.status='128,64,0'; return true"><AREA SHAPE=RECT
COORDS="1,15,18,33" ALT="255,255,255"  href="javascript:ChangeIMColor('255,255,255')"
OnMouseOver="window.status='255,255,255'; return true">
<AREA SHAPE=RECT COORDS="18,16,34,33" ALT="192,192,192"  href="javascript:ChangeIMColor('192,192,192')"
OnMouseOver="window.status='192,192,192'; return true"><AREA SHAPE=RECT
COORDS="35,16,50,33" ALT="255,0,0"    href="javascript:ChangeIMColor('255,0,0')"
OnMouseOver="window.status='255,0,0'; return true"><AREA SHAPE=RECT
COORDS="51,16,65,33" ALT="255,255,0"    href="javascript:ChangeIMColor('255,255,0')"
OnMouseOver="window.status='255,255,0'; return true"><AREA SHAPE=RECT
COORDS="67,16,82,32" ALT="0,255,0"    href="javascript:ChangeIMColor('0,255,0')"
OnMouseOver="window.status='0,255,0'; return true">
<AREA SHAPE=RECT COORDS="83,17,97,34" ALT="0,255,255"   href="javascript:ChangeIMColor('0,255,255')"
OnMouseOver="window.status='0,255,255'; return true"><AREA SHAPE=RECT 
COORDS="98,16,112,33" ALT="0,0,255"   href="javascript:ChangeIMColor('0,0,255')"
OnMouseOver="window.status='0,0,255'; return true"><AREA SHAPE=RECT
COORDS="114,16,130,33" ALT="255,0,255"   href="javascript:ChangeIMColor('255,0,255')"
OnMouseOver="window.status='255,0,255'; return true">
<AREA SHAPE=RECT COORDS="133,17,145,35" ALT="255,255,128"
  href="javascript:ChangeIMColor('255,255,128')"  OnMouseOver="window.status='255,255,128'; return
true"><AREA SHAPE=RECT COORDS="148,16,162,33" ALT="128,255,128"
  href="javascript:ChangeIMColor('128,255,128')"    OnMouseOver="window.status='128,255,128'; return
true"><AREA SHAPE=RECT COORDS="162,17,178,34" ALT="128,255,255"
  href="javascript:ChangeIMColor('128,255,255')"     OnMouseOver="window.status='128,255,255'; return true">
<AREA SHAPE=RECT COORDS="179,16,192,35" ALT="128,128,255" 
   href="javascript:ChangeIMColor('128,128,255')"    OnMouseOver="window.status='128,128,255'; return
true"><AREA SHAPE=RECT COORDS="196,16,210,32" ALT="255,0,128" 
   href="javascript:ChangeIMColor('255,0,128')"    OnMouseOver="window.status='255,0,128'; return
true"><AREA SHAPE=RECT COORDS="210,16,224,32" ALT="255,128,64" 
   href="javascript:ChangeIMColor('255,128,64')"    OnMouseOver="window.status='255,128,64'; return true">
</MAP></center><center></font>
</td>
</tr>
</table></center>

<center><table BORDER=1 COLS=1 WIDTH="237" BGCOLOR="#CCCCCC" >
<tr>
<td>
<center><b><font face="Times"><font color="#000000">Text 
Color</font></font></b>
</center><center><input type=text size="11" length="11" maxlength="11" 
name="textcolor" value="<?php echo $textcolor; ?>"></center><center><IMG 
SRC="palette.gif" USEMAP="#palette4.gif" WIDTH=228 HEIGHT=36 BORDER=0>
<MAP NAME="palette4.gif">
<AREA SHAPE=RECT COORDS="2,1,16,17" ALT="0,0,0" href="javascript:ChangeTextColor('0,0,0');"
OnMouseOver="window.status='0,0,0'; return true"
OnMouseOut="window.status='Color Palette'; return true"><AREA SHAPE=RECT
COORDS="17,0,32,16" ALT="128,128,128" href="javascript:ChangeTextColor('128,128,128')"
OnMouseOver="window.status='128,128,128'; return true"
OnMouseOut="window.status='Color Palette'; return true"><AREA SHAPE=RECT
COORDS="32,0,49,16" ALT="128,0,64"  href="javascript:ChangeTextColor('128,0,64')"
OnMouseOver="window.status='128,0,64'; return true"><AREA SHAPE=RECT
COORDS="49,1,64,15" ALT="128,128,0"  href="javascript:ChangeTextColor('128,128,0')"
OnMouseOver="window.status='128,128,0'; return true">
<AREA SHAPE=RECT COORDS="67,0,80,17" ALT="0,128,64" href="javascript:ChangeTextColor('0,128,64')"
OnMouseOver="window.status='0,128,64'; return true"><AREA SHAPE=RECT
COORDS="80,1,96,18" ALT="0,128,128" href="javascript:ChangeTextColor('0,128,128')"
OnMouseOver="window.status='0,128,128'; return true"><AREA SHAPE=RECT
COORDS="96,1,112,16" ALT="0,0,128"  href="javascript:ChangeTextColor('0,0,128')"
OnMouseOver="window.status='0,0,128'; return true"><AREA SHAPE=RECT 
COORDS="112,1,127,16" ALT="128,0,128" href="javascript:ChangeTextColor('128,0,128')"
OnMouseOver="window.status='128,0,128'; return true">
<AREA SHAPE=RECT COORDS="129,0,144,17" ALT="128,128,64" href="javascript:ChangeTextColor('128,128,64')"
OnMouseOver="window.status='128,128,64'; return true"><AREA SHAPE=RECT
COORDS="145,0,161,16" ALT="0,64,64"   href="javascript:ChangeTextColor('0,64,64')"
OnMouseOver="window.status='0,64,64'; return true"><AREA SHAPE=RECT
COORDS="161,1,176,16" ALT="0,128,255"   href="javascript:ChangeTextColor('0,128,255')"
OnMouseOver="window.status='0,128,255'; return true"><AREA SHAPE=RECT
COORDS="178,2,192,16" ALT="0,64,128"    href="javascript:ChangeTextColor('0,64,128')"
OnMouseOver="window.status='0,64,128'; return true">
<AREA SHAPE=RECT COORDS="195,2,209,15" ALT="64,0,255"      href="javascript:ChangeTextColor('64,0,255')"
OnMouseOver="window.status='64,0,255'; return true"><AREA SHAPE=RECT
COORDS="211,2,225,15" ALT="128,64,0"        href="javascript:ChangeTextColor('128,64,0')"
OnMouseOver="window.status='128,64,0'; return true"><AREA SHAPE=RECT
COORDS="1,15,18,33" ALT="255,255,255"  href="javascript:ChangeTextColor('255,255,255')"
OnMouseOver="window.status='255,255,255'; return true">
<AREA SHAPE=RECT COORDS="18,16,34,33" ALT="192,192,192"  href="javascript:ChangeTextColor('192,192,192')"
OnMouseOver="window.status='192,192,192'; return true"><AREA SHAPE=RECT
COORDS="35,16,50,33" ALT="255,0,0"    href="javascript:ChangeTextColor('255,0,0')"
OnMouseOver="window.status='255,0,0'; return true"><AREA SHAPE=RECT
COORDS="51,16,65,33" ALT="255,255,0"    href="javascript:ChangeTextColor('255,255,0')"
OnMouseOver="window.status='255,255,0'; return true"><AREA SHAPE=RECT
COORDS="67,16,82,32" ALT="0,255,0"    href="javascript:ChangeTextColor('0,255,0')"
OnMouseOver="window.status='0,255,0'; return true">
<AREA SHAPE=RECT COORDS="83,17,97,34" ALT="0,255,255"   href="javascript:ChangeTextColor('0,255,255')"
OnMouseOver="window.status='0,255,255'; return true"><AREA SHAPE=RECT 
COORDS="98,16,112,33" ALT="0,0,255"   href="javascript:ChangeTextColor('0,0,255')"
OnMouseOver="window.status='0,0,255'; return true"><AREA SHAPE=RECT
COORDS="114,16,130,33" ALT="255,0,255"   href="javascript:ChangeTextColor('255,0,255')"
OnMouseOver="window.status='255,0,255'; return true">
<AREA SHAPE=RECT COORDS="133,17,145,35" ALT="255,255,128"
  href="javascript:ChangeTextColor('255,255,128')"  OnMouseOver="window.status='255,255,128'; return
true"><AREA SHAPE=RECT COORDS="148,16,162,33" ALT="128,255,128"
  href="javascript:ChangeTextColor('128,255,128')"    OnMouseOver="window.status='128,255,128'; return
true"><AREA SHAPE=RECT COORDS="162,17,178,34" ALT="128,255,255"
  href="javascript:ChangeTextColor('128,255,255')"     OnMouseOver="window.status='128,255,255'; return true">
<AREA SHAPE=RECT COORDS="179,16,192,35" ALT="128,128,255" 
   href="javascript:ChangeTextColor('128,128,255')"    OnMouseOver="window.status='128,128,255'; return
true"><AREA SHAPE=RECT COORDS="196,16,210,32" ALT="255,0,128" 
   href="javascript:ChangeTextColor('255,0,128')"    OnMouseOver="window.status='255,0,128'; return
true"><AREA SHAPE=RECT COORDS="210,16,224,32" ALT="255,128,64" 
   href="javascript:ChangeTextColor('255,128,64')"    OnMouseOver="window.status='255,128,64'; return true">
</MAP></center><center></font>
</td>
</tr>
</table>

<center><table BORDER=1 WIDTH="237" BGCOLOR="#CCCCCC" >
<tr>
<td ALIGN=CENTER VALIGN=TOP><b><font face="Times"><font 
color="#000000">Image Shading   
Color</font></font></b>
</center><center><input type=text size="11" length="11" maxlength="11" 
name="ishadingcolor" value="<?php echo $ishadingcolor; 
?>"></center><center><IMG SRC="palette.gif" USEMAP="#palette5.gif" WIDTH=228 
HEIGHT=36 BORDER=0>
<MAP NAME="palette5.gif">
<AREA SHAPE=RECT COORDS="2,1,16,17" ALT="0,0,0" href="javascript:ChangeIshade('0,0,0');"
OnMouseOver="window.status='0,0,0'; return true"
OnMouseOut="window.status='Color Palette'; return true"><AREA SHAPE=RECT
COORDS="17,0,32,16" ALT="128,128,128" href="javascript:ChangeIshade('128,128,128')"
OnMouseOver="window.status='128,128,128'; return true"
OnMouseOut="window.status='Color Palette'; return true"><AREA SHAPE=RECT
COORDS="32,0,49,16" ALT="128,0,64"  href="javascript:ChangeIshade('128,0,64')"
OnMouseOver="window.status='128,0,64'; return true"><AREA SHAPE=RECT
COORDS="49,1,64,15" ALT="128,128,0"  href="javascript:ChangeIshade('128,128,0')"
OnMouseOver="window.status='128,128,0'; return true">
<AREA SHAPE=RECT COORDS="67,0,80,17" ALT="0,128,64" href="javascript:ChangeIshade('0,128,64')"
OnMouseOver="window.status='0,128,64'; return true"><AREA SHAPE=RECT
COORDS="80,1,96,18" ALT="0,128,128" href="javascript:ChangeIshade('0,128,128')"
OnMouseOver="window.status='0,128,128'; return true"><AREA SHAPE=RECT
COORDS="96,1,112,16" ALT="0,0,128"  href="javascript:ChangeIshade('0,0,128')"
OnMouseOver="window.status='0,0,128'; return true"><AREA SHAPE=RECT 
COORDS="112,1,127,16" ALT="128,0,128" href="javascript:ChangeIshade('128,0,128')"
OnMouseOver="window.status='128,0,128'; return true">
<AREA SHAPE=RECT COORDS="129,0,144,17" ALT="128,128,64" href="javascript:ChangeIshade('128,128,64')"
OnMouseOver="window.status='128,128,64'; return true"><AREA SHAPE=RECT
COORDS="145,0,161,16" ALT="0,64,64"   href="javascript:ChangeIshade('0,64,64')"
OnMouseOver="window.status='0,64,64'; return true"><AREA SHAPE=RECT
COORDS="161,1,176,16" ALT="0,128,255"   href="javascript:ChangeIshade('0,128,255')"
OnMouseOver="window.status='0,128,255'; return true"><AREA SHAPE=RECT
COORDS="178,2,192,16" ALT="0,64,128"    href="javascript:ChangeIshade('0,64,128')"
OnMouseOver="window.status='0,64,128'; return true">
<AREA SHAPE=RECT COORDS="195,2,209,15" ALT="64,0,255"      href="javascript:ChangeIshade('64,0,255')"
OnMouseOver="window.status='64,0,255'; return true"><AREA SHAPE=RECT
COORDS="211,2,225,15" ALT="128,64,0"        href="javascript:ChangeIshade('128,64,0')"
OnMouseOver="window.status='128,64,0'; return true"><AREA SHAPE=RECT
COORDS="1,15,18,33" ALT="255,255,255"  href="javascript:ChangeIshade('255,255,255')"
OnMouseOver="window.status='255,255,255'; return true">
<AREA SHAPE=RECT COORDS="18,16,34,33" ALT="192,192,192"  href="javascript:ChangeIshade('192,192,192')"
OnMouseOver="window.status='192,192,192'; return true"><AREA SHAPE=RECT
COORDS="35,16,50,33" ALT="255,0,0"    href="javascript:ChangeIshade('255,0,0')"
OnMouseOver="window.status='255,0,0'; return true"><AREA SHAPE=RECT
COORDS="51,16,65,33" ALT="255,255,0"    href="javascript:ChangeIshade('255,255,0')"
OnMouseOver="window.status='255,255,0'; return true"><AREA SHAPE=RECT
COORDS="67,16,82,32" ALT="0,255,0"    href="javascript:ChangeIshade('0,255,0')"
OnMouseOver="window.status='0,255,0'; return true">
<AREA SHAPE=RECT COORDS="83,17,97,34" ALT="0,255,255"   href="javascript:ChangeIshade('0,255,255')"
OnMouseOver="window.status='0,255,255'; return true"><AREA SHAPE=RECT 
COORDS="98,16,112,33" ALT="0,0,255"   href="javascript:ChangeIshade('0,0,255')"
OnMouseOver="window.status='0,0,255'; return true"><AREA SHAPE=RECT
COORDS="114,16,130,33" ALT="255,0,255"   href="javascript:ChangeIshade('255,0,255')"
OnMouseOver="window.status='255,0,255'; return true">
<AREA SHAPE=RECT COORDS="133,17,145,35" ALT="255,255,128"
  href="javascript:ChangeIshade('255,255,128')"  OnMouseOver="window.status='255,255,128'; return
true"><AREA SHAPE=RECT COORDS="148,16,162,33" ALT="128,255,128"
  href="javascript:ChangeIshade('128,255,128')"    OnMouseOver="window.status='128,255,128'; return
true"><AREA SHAPE=RECT COORDS="162,17,178,34" ALT="128,255,255"
  href="javascript:ChangeIshade('128,255,255')"     OnMouseOver="window.status='128,255,255'; return true">
<AREA SHAPE=RECT COORDS="179,16,192,35" ALT="128,128,255" 
   href="javascript:ChangeIshade('128,128,255')"    OnMouseOver="window.status='128,128,255'; return
true"><AREA SHAPE=RECT COORDS="196,16,210,32" ALT="255,0,128" 
   href="javascript:ChangeIshade('255,0,128')"    OnMouseOver="window.status='255,0,128'; return
true"><AREA SHAPE=RECT COORDS="210,16,224,32" ALT="255,128,64" 
   href="javascript:ChangeIshade('255,128,64')"    OnMouseOver="window.status='255,128,64'; return true">
</MAP></center><center></font>
</td>
</tr>
</table></center>

<table BORDER=1 COLS=1 WIDTH="237" BGCOLOR="#CCCCCC" >
<tr>
<td align=center>
<center> 
<a href="http://phpaint.sourceforge.net" target="Newwind">
<img src="phpimagebank.php?s=21&text=PHPaint+v.2.1&ts1=128&ts2=128&ts3=128&im1=192&im2=192&im3=212&te1=255&te2=255&te3=255&is1=128&is2=128&is3=148&font=times.ttf&xpad=4&ypad=6&fontdir=<? echo getcwd();?><? echo "/";?>" border=0 alt='PHPaint v.2.1'></a>
<br><font size=-1>Copyright 1999-2002 Erica Andrews.<br><a 
href="mailto:PhrozenSmoke@yahoo.com">PhrozenSmoke@yahoo.com</a><br><br><a 
href="phpaint_tutorial.html" target="tut">TUTORIAL</a> &nbsp; &nbsp; <a 
href="phpaint_manual.html" target="manual">Manual</a>
<br><a 
href="phpaint_manual.html#usageag">Usage Agreement</a>
</td>
</tr>
</table>

</center>
</td>
</tr>
</table>
</form>
</body>
</html>

