//
//  Centrallix Test Suite
//
//  "T01-Combined1.app"
//  By Luke Ehresman - August 23, 2002
//
//  This app tests the form and object source widgets and their
//  interaction with the dynamic table widget.  The information
//  is queried from States.csv and displayed for modification
//  and viewing in different form elements.
//
//  Widgets Used:
//    page, osrc, form, pane, table, htmlwindow, editbox,
//    label, connector
//

$Version=2$
Page1 "widget/page"
    {
    title="Test #01 - 1st Combined Page";
    background="/sys/images/wood.png";

    TextButton3 "widget/textbutton"
	{
	x=10; y=10; height=25; width=60;
	fgcolor1="#000000";
	fgcolor2="#c0c0c0";
	background="/sys/images/slate2.gif";
	text="INDEX";
	tristate="no";

	Connector5 "widget/connector" { event="Click"; target="Page1"; action="LoadPage"; Source="'index.app'"; }
	}

    OSRC1 "widget/osrc"
	{
	replicasize=50;
	readahead=15;
	sql="SELECT :id,:full_name,:abbrev FROM /tests/States.csv/rows";

	Form1 "widget/form"
	    {

	    Pane1 "widget/pane"
		{
		x=10; y=40; width=406; height=224;
		background="/sys/images/slate2.gif";
		style="raised";

		Pane2 "widget/pane"
		    {
		    x=10; y=10; width=384; height=202;
		    style="lowered";
		    bgcolor="#c0c0c0";

		    Table1 "widget/table"
			{
			mode="dynamicrow";
			x=0; y=0; width=382; height=200;
			windowsize=10;
			cellhspacing=0;
			cellvspacing=0;
			rowheight=20;
			inner_border=0;
			inner_padding=2;
			bgcolor="#c0c0c0";
			row_bgcolor1="#a0a0a0";
			row_bgcolor2="#a0a0a0";
			row_bgcolorhighlight="#909090";
			hdr_bgcolor="#c0c0c0";
			textcolor="black";
			textcolorhighlight="black";
			titlecolor="#000000";

			id "widget/table-column" { title="<b>ID</b>"; width=50; }
			abbrev "widget/table-column" { title="<b>Abbrev.</b>"; width=60; }
			full_name "widget/table-column" { title="<b>Full State Name</b>"; width=250; }

			Connector1 "widget/connector" { event="Click"; target="Window1"; action="SetVisibility"; IsVisible=1; }
			}
		    }
		}
	
	    Window1 "widget/htmlwindow"
		{
		x=60; y=60; height=110; width=280;
		hdr_bgcolor="#ffcc00";
		bgcolor="#c0c0c0";
		style="dialog";
		visible="false";
		title="State Information";

		// State Name
		Label1 "widget/label"
		    {
		    x=3; y=3; width=80; height=15;
		    text="State Name:";
		    align="right";
		    }
		Editbox1 "widget/editbox"
		    {
		    x=90; y=5; width=160; height=15;
		    style="lowered";
		    fieldname="full_name";
		    bgcolor="#ffffff";
		    }

		// State Abbreviation
		Label2 "widget/label"
		    {
		    x=3; y=23; width=80; height=15;
		    text="Abbreviation:";
		    align="right";
		    }
		Editbox2 "widget/editbox"
		    {
		    x=90; y=25; width=24; height=15;
		    style="lowered";
		    fieldname="abbrev";
		    bgcolor="#ffffff";
		    }

		// Buttons
		TextButton1 "widget/textbutton"
		    {
		    x=90; y=50; height=25; width=60;
		    fgcolor1="#000000";
		    fgcolor2="#c0c0c0";
		    text="Save";
		    tristate="no";

		    Connector2 "widget/connector" { event="Click"; target="Form1"; action="Save"; }
		    Connector3 "widget/connector" { event="Click"; target="Window1"; action="SetVisibility"; IsVisible=0; }
		    }
		TextButton2 "widget/textbutton"
		    {
		    x=155; y=50; height=25; width=60;
		    fgcolor1="#000000";
		    fgcolor2="#c0c0c0";
		    text="Cancel";
		    tristate="no";

		    Connector4 "widget/connector" { event="Click"; target="Window1"; action="SetVisibility"; IsVisible=0; }
		    }
		}
	    }
	}
    }