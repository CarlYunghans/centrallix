$Version=2$
ors "widget/page"
    {
    title = "Obscure/Reveal Subsystem Demonstration App";
    bgcolor='#606060';

    ConfirmWindow "widget/htmlwindow"
	{
	title = "Data Was Modified!";
	titlebar = yes;
	hdr_bgcolor="#c00000";
	bgcolor= "#e0e0e0";
	visible = false;
	style = dialog;
	x=200;y=200;width=300;height=140;

	warninglabel "widget/label"
	    {
	    x=10;y=10;width=276;height=30;
	    text="Some data was modified.  Do you want to save it first, discard your modifications, or simply cancel the operation?";
	    }

	_3bConfirmSave "widget/textbutton"
	    {
	    x=10;y=75;width=80;height=30;
	    tristate=no;
	    background="/sys/images/grey_gradient.png";
	    text = "Save";
	    fgcolor1=black;fgcolor2=white;
	    }
	_3bConfirmDiscard "widget/textbutton"
	    {
	    x=110;y=75;width=80;height=30;
	    tristate=no;
	    background="/sys/images/grey_gradient.png";
	    text = "Discard";
	    fgcolor1=black;fgcolor2=white;
	    }
	_3bConfirmCancel "widget/textbutton"
	    {
	    x=210;y=75;width=80;height=30;
	    tristate=no;
	    background="/sys/images/grey_gradient.png";
	    text = "Cancel";
	    fgcolor1=black;fgcolor2=white;
	    }
	}

    win1btn "widget/textbutton"
	{
	x=10;y=10;width=60;height=120;
	text='Win1';
	tristate=no;
	bgcolor='#e0e0e0';
	fgcolor1='black';
	fgcolor2='white';
	cn1 "widget/connector" { event=Click; target=win1; action=SetVisibility; }
	}
    win2btn "widget/textbutton"
	{
	x=10;y=140;width=60;height=120;
	text='Win2';
	tristate=no;
	bgcolor='#e0e0e0';
	fgcolor1='black';
	fgcolor2='white';
	cn2 "widget/connector" { event=Click; target=win2; action=SetVisibility; }
	}
    win1 "widget/htmlwindow"
	{
	x=80;y=10;width=350;height=250;
	hdr_bgcolor='#e0e0e0';
	bgcolor='#c0c0c0';
	style=dialog;

	osrc1 "widget/osrc"
	    {
	    sql = "SELECT :id, :full_name, :short_name, :num_days, :num_leapyear_days FROM /samples/Months.csv/rows";
	    baseobj = "/samples/Months.csv/rows";
	    readahead=1;
	    replicasize=4;
	    autoquery = onfirstreveal;
	    form1 "widget/form"
		{
		_3bconfirmwindow = "ConfirmWindow";

		formctlpane "widget/pane"
		    {
		    x = 100; y=10; width=240; height=30;
		    style=raised;
		    //bgcolor="#c0c0c0";
		    background="/sys/images/grey_gradient3.png";

		    formstatus "widget/formstatus"
			{
			x=72;y=4;
			style=largeflat;
			}
		    btnFirst "widget/imagebutton"
			{
			x=8;y=5;
			width=18; height=18;
			image = "/sys/images/ico16aa.gif";
			pointimage = "/sys/images/ico16ab.gif";
			clickimage = "/sys/images/ico16ac.gif";
			disabledimage = "/sys/images/ico16ad.gif";
			enabled = runclient(:form1:recid > 1);
			cn1 "widget/connector"
			    {
			    event="Click";
			    target="form1";
			    action="First";
			    }
			}
		    btnPrev "widget/imagebutton"
			{
			x=28;y=5;
			width=18; height=18;
			image = "/sys/images/ico16ba.gif";
			pointimage = "/sys/images/ico16bb.gif";
			clickimage = "/sys/images/ico16bc.gif";
			disabledimage = "/sys/images/ico16bd.gif";
			cn1 "widget/connector"
			    {
			    event="Click";
			    target="form1";
			    action="Prev";
			    }
			enabled = runclient(:form1:recid > 1);
			}
		    btnNext "widget/imagebutton"
			{
			x=190;y=5;
			width=18; height=18;
			image = "/sys/images/ico16ca.gif";
			pointimage = "/sys/images/ico16cb.gif";
			clickimage = "/sys/images/ico16cc.gif";
			disabledimage = "/sys/images/ico16cd.gif";
			cn1 "widget/connector"
			    {
			    event="Click";
			    target="form1";
			    action="Next";
			    }
			enabled = runclient(not (:form1:recid == :form1:lastrecid));
			}
		    btnLast "widget/imagebutton"
			{
			x=210;y=5;
			width=18; height=18;
			image = "/sys/images/ico16da.gif";
			pointimage = "/sys/images/ico16db.gif";
			clickimage = "/sys/images/ico16dc.gif";
			disabledimage = "/sys/images/ico16dd.gif";
			cn1 "widget/connector"
			    {
			    event="Click";
			    target="form1";
			    action="Last";
			    }
			enabled = runclient(not (:form1:recid == :form1:lastrecid));
			}
		    }

		searchbtn "widget/textbutton"
		    {
		    x = 10; y=10; height=30; width=70;
		    tristate=no;
		    background="/sys/images/grey_gradient.png";
		    text="Search";
		    enabled = runclient(:form1:is_queryable or :form1:is_queryexecutable);
		    fgcolor1=black;fgcolor2=white;
		    cn1 "widget/connector" { event="Click"; target="form1"; action="QueryToggle"; }
		    }
		newbtn "widget/textbutton"
		    {
		    x = 10; y=45; height=30; width=70;
		    tristate=no;
		    background="/sys/images/grey_gradient.png";
		    text="New";
		    enabled = runclient(:form1:is_newable);
		    fgcolor1=black;fgcolor2=white;
		    cn1 "widget/connector" { event="Click"; target="form1"; action="New"; }
		    }
		editbtn "widget/textbutton"
		    {
		    x = 10; y=80; height=30; width=70;
		    tristate=no;
		    background="/sys/images/grey_gradient.png";
		    text="Edit";
		    enabled = runclient(:form1:is_editable);
		    fgcolor1=black;fgcolor2=white;
		    cn1 "widget/connector" { event="Click"; target="form1"; action="Edit"; }
		    }
		savebtn "widget/textbutton"
		    {
		    x = 10; y=115; height=30; width=70;
		    tristate=no;
		    background="/sys/images/grey_gradient.png";
		    text="Save";
		    enabled = runclient(:form1:is_savable);
		    fgcolor1=black;fgcolor2=white;
		    cn1 "widget/connector" { event="Click"; target="form1"; action="Save"; }
		    }
		cancelbtn "widget/textbutton"
		    {
		    x = 10; y=150; height=30; width=70;
		    tristate=no;
		    background="/sys/images/grey_gradient.png";
		    text="Cancel";
		    enabled = runclient(:form1:is_discardable);
		    fgcolor1=black;fgcolor2=white;
		    cn1 "widget/connector" { event="Click"; target="form1"; action="Discard"; }
		    }
		delbtn "widget/textbutton"
		    {
		    x = 10; y=185; height=30; width=70;
		    tristate=no;
		    background="/sys/images/grey_gradient.png";
		    text="Delete";
		    enabled = runclient(:form1:is_editable);
		    fgcolor1=black;fgcolor2=white;
		    cn1 "widget/connector" { event="Click"; target="form1"; action="Delete"; }
		    }

		id_label "widget/label"
		    {
		    x=100;y=45;width=130; height=20;
		    text="ID:";
		    align=right;
		    }
		id "widget/editbox"
		    {
		    style=lowered;
		    bgcolor=white;
		    x=240;y=45;width=100;height=20;
		    fieldname="id";
		    }

		fullname_label "widget/label"
		    {
		    x=100;y=73;width=130; height=20;
		    text="Month Name:";
		    align=right;
		    }
		fullname "widget/editbox"
		    {
		    style=lowered;
		    bgcolor=white;
		    x=240;y=73;width=100;height=20;
		    fieldname="full_name";
		    }

		shortname_label "widget/label"
		    {
		    x=100;y=101;width=130; height=20;
		    text="Abbreviation:";
		    align=right;
		    }
		shortname "widget/editbox"
		    {
		    style=lowered;
		    bgcolor=white;
		    x=240;y=101;width=100;height=20;
		    fieldname="short_name";
		    }

		days_label "widget/label"
		    {
		    x=100;y=129;width=130; height=20;
		    text="Days:";
		    align=right;
		    }
//		days "widget/editbox"
//		    {
//		    style=lowered;
//		    bgcolor=white;
//		    x=240;y=129;width=100;height=20;
//		    fieldname="num_days";
//		    }
		days_dd "widget/dropdown"
		    {
		    mode=static;
		    bgcolor="#c0c0c0";
		    x=240;y=129;width=100;height=20;
		    hilight="#000080";
		    fieldname="num_days";
		    days_dd_1 "widget/dropdownitem" { label="28 Days"; value="28"; }
		    days_dd_2 "widget/dropdownitem" { label="30 Days"; value="30"; }
		    days_dd_3 "widget/dropdownitem" { label="31 Days"; value="31"; }
		    }

		ldays_label "widget/label"
		    {
		    x=100;y=157;width=130; height=20;
		    text="Leapyear Days:";
		    align=right;
		    }
		ldays "widget/editbox"
		    {
		    style=lowered;
		    bgcolor=white;
		    x=240;y=157;width=100;height=20;
		    fieldname="num_leapyear_days";
		    }
		}
	    }
	}

    tabtextbtn1 "widget/textbutton"
	{
	x=10;y=300;width=60;height=40;
	bgcolor='#e0e0e0';
	tristate=no;
	text='Tab 1';
	fgcolor1=black;
	fgcolor2=white;
	cnt1 "widget/connector" { event=Click; target=tab1; action=SetTab; Tab="'PageOne'"; }
	}
    tabtextbtn2 "widget/textbutton"
	{
	x=10;y=357;width=60;height=40;
	bgcolor='#e0e0e0';
	tristate=no;
	text='Tab 2';
	fgcolor1=black;
	fgcolor2=white;
	cnt2 "widget/connector" { event=Click; target=tab1; action=SetTab; Tab="'PageTwo'"; }
	}
    tabtextbtn3 "widget/textbutton"
	{
	x=10;y=414;width=60;height=40;
	bgcolor='#e0e0e0';
	tristate=no;
	text='Tab 3';
	fgcolor1=black;
	fgcolor2=white;
	cnt3 "widget/connector" { event=Click; target=tab1; action=SetTab; Tab="'PageThree'"; }
	}

    tab1 "widget/tab"
	{
	x=80;y=300;width=350;height=130;
	bgcolor='#c0c0c0';
	PageOne "widget/tabpage"
	    {
	    osrc2 "widget/osrc"
		{
		sql = "SELECT :first_name, :last_name, :email FROM /samples/people.csv/rows";
		baseobj = "/samples/people.csv/rows";
		readahead=1;
		replicasize=4;
		autoquery = onfirstreveal;

		form2 "widget/form"
		    {
		    _3bconfirmwindow = ConfirmWindow;
		    form2status "widget/formstatus" { x=5;y=5; style=largeflat; }
		    firstname_label "widget/label" { x=5;y=40;width=70; height=20; text="First Name:"; align=right; }
		    firstname "widget/editbox" { style=lowered; bgcolor=white; x=80;y=40;width=100;height=20; fieldname="first_name"; }
		    }
		}
	    }
	PageTwo "widget/tabpage"
	    {
	    osrc3 "widget/osrc"
		{
		sql = "SELECT :first_name, :computer_name, :memory FROM /samples/computers.csv/rows";
		baseobj = "/samples/computers.csv/rows";
		readahead=1;
		replicasize=4;
		autoquery = onfirstreveal;

		form3 "widget/form"
		    {
		    _3bconfirmwindow = ConfirmWindow;
		    form3status "widget/formstatus" { x=5;y=5; style=largeflat; }
		    compname_label "widget/label" { x=5;y=40;width=70; height=20; text="Computer:"; align=right; }
		    compname "widget/editbox" { style=lowered; bgcolor=white; x=80;y=40;width=100;height=20; fieldname="computer_name"; }
		    }
		}
	    }
	PageThree "widget/tabpage"
	    {
	    }
	}

    win2 "widget/htmlwindow"
	{
	x=440;y=10;width=320;height=250;
	hdr_bgcolor='#e0e0e0';
	bgcolor='#c0c0c0';
	style=dialog;

	tab2 "widget/tab"
	    {
	    x=10;y=10;width=298;height=180;
	    bgcolor="#a0a0a0";

	    tab2p1 "widget/tabpage"
		{
		osrc4 "widget/osrc"
		    {
		    sql = "SELECT :fileid, :computer_name, :parentid, :file_name FROM /samples/files.csv/rows";
		    baseobj = "/samples/files.csv/rows";
		    readahead=1;
		    replicasize=4;
		    autoquery = oneachreveal;

		    form4 "widget/form"
			{
			_3bconfirmwindow = ConfirmWindow;
			form4status "widget/formstatus" { x=5;y=5; style=largeflat; }
			filename_label "widget/label" { x=5;y=40;width=70; height=20; text="File Name:"; align=right; }
			filename "widget/editbox" { style=lowered; bgcolor=white; x=80;y=40;width=100;height=20; fieldname="file_name"; }
			}
		    }
		}
	    tab2p2 "widget/tabpage"
		{
		}
	    }
	}

    debugwin "widget/htmlwindow"
	{
	x=600;y=220;width=600;height=434;
	hdr_bgcolor='#e0e0e0';
	bgcolor='#c0c0c0';
	visible=true;
	style=dialog;
	Treeview_pane "widget/pane"
	    {
	    x=1; y=1; width=596; height=235;
	    style=lowered;
	    Tree_scroll "widget/scrollpane"
		{
		x=0; y=0; width=594; height=233;
		Tree "widget/treeview"
		    {
		    x=0; y=1; width=574;
		    source = "javascript:osrc1";
		    }
		}
	    }
	Log_pane "widget/pane"
	    {
	    x=1;y=236;width=596;height=150;
	    style=lowered;
	    Log_scroll "widget/scrollpane"
		{
		x=0;y=0;width=594;height=148;
		Log "widget/html"
		    {
		    x=0;y=0;width=574;
		    source = "debug:";
		    mode=dynamic;
		    }
		}
	    }
	Show_btn "widget/textbutton"
	    {
	    x=1;y=387;width=80;height=20;
	    tristate=no;
	    fgcolor1="black";
	    fgcolor2="white";
	    text="Show Log";
	    showconn "widget/connector" { event=Click; target=Log; action=ShowText; }
	    }
	}
    }