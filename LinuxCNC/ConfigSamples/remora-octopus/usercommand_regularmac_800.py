import linuxcnc
s = linuxcnc.stat()
s.poll()
rC = root_window.tk.call
rE = root_window.tk.eval

############################################
########     COPY LOTS FROM # PLASMAC2     #########
##############################################

spinBoxes = []
toolButtons  = ['machine_estop','machine_power','file_open','reload','program_run',
                'program_step','program_pause','program_stop','program_blockdelete',
                'program_optpause','view_zoomin','view_zoomout','view_z','view_z2',
                'view_x','view_y','view_y2','view_p','rotate','clear_plot']

configPath = os.getcwd()
#configPath = " " 
##############################################################################
# NEW CLASSES                                                                #
##############################################################################
# class for preferences file
prefP = configparser.ConfigParser
class plasmacPreferences(prefP):   # PLASMAC2
    optionxform = str
    types = {bool: prefP.getboolean,
             float: prefP.getfloat,
             int: prefP.getint,
             str: prefP.get,
             repr: lambda self, section, option: eval(prefP.get(self, section, option)),
            }

    def __init__(self):
        prefP.__init__(self, strict=False, interpolation=None)
        self.fn = os.path.join(configPath, '{}.prefs'.format(vars.machine.get()))
        self.read(self.fn)
  
##############################################################################
# PREFERENCE FUNCTIONS                                                       #
##############################################################################
def getPrefs(prefs, section, option, default=False, type=bool):                 # PLASMAC2
    m = prefs.types.get(type)
    if prefs.has_section(section):
        if prefs.has_option(section, option):
            return m(prefs, section, option)
        else:
            prefs.set(section, option, str(default))
            prefs.write(open(prefs.fn, 'w'))
            return default
    else:
        prefs.add_section(section)
        prefs.set(section, option, str(default))
        prefs.write(open(prefs.fn, 'w'))
        return default

def putPrefs(prefs, section, option, value, type=bool):                 # PLASMAC2
    if prefs.has_section(section):
        prefs.set(section, option, str(type(value)))
        prefs.write(open(prefs.fn, 'w'))
    else:
        prefs.add_section(section)
        prefs.set(section.upper(), option, str(type(value)))
        prefs.write(open(prefs.fn, 'w'))

def removePrefsSect(prefs, section):            # PLASMAC2
    prefs.remove_section(section)
    prefs.write(open(prefs.fn, 'w'))

def sortPrefs(prefs):           # PLASMAC2
    prefs._sections = OrderedDict(sorted(prefs._sections.items(), key=lambda t: int(t[0].rsplit('_',1)[1]) ))
    prefs.write(open(prefs.fn, 'w'))
PREF = plasmacPreferences()


# class for popup dialogs           # PLASMAC2
class plasmacDialog:                
    def __init__(self, func, title, msg, system=None):
        dlg = self.dlg = Tkinter.Toplevel(root_window, bg=colorBack)
        dlg.attributes('-type', 'dock')
        rE('tk::PlaceWindow {} center'.format(dlg))
        dlg.wait_visibility()
        dlg.grab_set()
        dlg.protocol("WM_DELETE_WINDOW", lambda:self.dlg_complete(False, False))
        dlg.title(title)
        frm = Tkinter.Frame(dlg, bg=colorBack, bd=2, relief='flat')
        ttl = Tkinter.Label(frm, text=title, fg=colorBack, bg=colorFore)
        ttl.pack(fill='x')
        if func == 'rfl':
            self.leadIn = Tkinter.BooleanVar()
            self.leadLength = Tkinter.StringVar()
            self.leadAngle = Tkinter.StringVar()
            f1 = Tkinter.Frame(frm, bg=colorBack)
            lbl1 = Tkinter.Label(f1, text=_('Use Leadin:'), fg=colorFore, bg=colorBack, width=12, anchor='e')
            lbl1.pack(side='left')
            leadinDo = Tkinter.Checkbutton(f1, fg=colorFore, bg=colorBack, variable=self.leadIn, indicatoron=False, width=2, bd=1)
            leadinDo.configure(highlightthickness=0, activebackground=colorBack, selectcolor=colorActive, relief='raised', overrelief='raised')
            leadinDo.pack(side='left')
            f1.pack(padx=4, pady=4, anchor='w')
            f2 = Tkinter.Frame(frm, bg=colorBack)
            lbl2 = Tkinter.Label(f2, text=_('Leadin Length:'), fg=colorFore, bg=colorBack, width=12, anchor='e')
            lbl2.pack(side='left')
            leadinLength = Tkinter.Spinbox(f2, fg=colorFore, bg=colorBack, textvariable=self.leadLength, width=10)
            leadinLength.configure(font=(fontName, fontSize), highlightthickness=0)
            leadinLength.pack(side='left')
            f2.pack(padx=4, pady=4, anchor='w')
            f3 = Tkinter.Frame(frm, bg=colorBack)
            lbl3 = Tkinter.Label(f3, text=_('Leadin Angle:'), fg=colorFore, bg=colorBack, width=12, anchor='e')
            lbl3.pack(side='left')
            leadinAngle = Tkinter.Spinbox(f3, fg=colorFore, bg=colorBack, textvariable=self.leadAngle, width=10)
            leadinAngle.configure(font=(fontName, fontSize), highlightthickness=0)
            leadinAngle.pack(side='left')
            f3.pack(padx=4, pady=4, anchor='w')
            self.leadIn.set(False)
            if s.linear_units == 1:
                leadinLength.config(width=10, from_=1, to=25, increment=1, format='%0.0f', wrap=1)
                self.leadLength.set(5)
            else:
                leadinLength.config(width=10, from_=0.05, to=1, increment=0.05, format='%0.2f', wrap=1)
                self.leadLength.set(0.2)
            leadinAngle.config(width=10, from_=-359, to=359, increment=1, format='%0.0f', wrap=1)
            self.leadAngle.set(0)
        else:
            label = Tkinter.Label(frm, text=msg, fg=colorFore, bg=colorBack)
            label.pack(padx=4, pady=4)
        if func in ['entry', 'touch']:
            self.entry = Tkinter.Entry(frm, justify='right', fg=colorFore, bg=colorBack)
            self.entry.configure(highlightthickness=0, selectforeground=colorBack, selectbackground=colorFore)
            self.entry.pack(padx=4, pady=4)
            self.entry.focus_set()
        if func == 'touch':
            self.entry.insert('end', '0.0')
            opl = Tkinter.Label(frm, text=_('Coordinate System'), fg=colorFore, bg=colorBack)
            opl.pack(padx=4, pady=4)
            self.c = c = StringVar(t)
            c.set(system)
            self.opt = Tkinter.OptionMenu(frm, c, *all_systems[:])
            self.opt.configure(fg=colorFore, bg=colorBack, activebackground=colorBack, highlightthickness=0)
            self.opt.children['menu'].configure(fg=colorFore, bg=colorBack, activeforeground=colorBack, activebackground=colorFore)
            self.opt.pack(padx=4, pady=4)
        bbox = Tkinter.Frame(frm, bg=colorBack)
        if func == 'rfl':
            b1Text = _('Load')
            b2Text = _('Cancel')
        if func in ['info', 'error', 'warn']:
            b1Text = _('OK')
            b2Text = None
        elif func in ['yesno']:
            b1Text = _('Yes')
            b2Text = _('No')
        elif func in ['entry', 'touch']:
            b1Text = _('OK')
            b2Text = _('Cancel')
        b1 = Tkinter.Button(bbox, text=b1Text, command=lambda:self.dlg_complete(True, func), width=8)
        b1.configure(fg=colorFore, bg=colorBack, activebackground=colorBack, highlightthickness=0)
        b1.pack(side='left')
        if b2Text:
            b2 = Tkinter.Button(bbox, text=b2Text, command=lambda:self.dlg_complete(False, func), width=8)
            b2.configure(fg=colorFore, bg=colorBack, activebackground=colorBack, highlightthickness=0)
            b2.pack(side='left', padx=(8,0))
        bbox.pack(padx=4, pady=4)
        frm.pack()

    def dlg_complete(self, value, func):
        if func == 'rfl':
            self.reply = value, self.leadIn.get(), float(self.leadLength.get()), float(self.leadAngle.get())
        elif func in ['entry']:
#            text = None if not self.entry.get() else self.entry.get()
            self.reply = value, self.entry.get()
        elif func in ['touch']:
#            text = None if not self.entry.get() else self.entry.get()
            self.reply = value, self.entry.get(), self.c.get()
        else:
            self.reply = value
        self.dlg.destroy()


##################################
#######   GCODE PRE     ###################
########################################

def auto_tab_raise():
    #pVars.editmode.set(True)
    #print(pVars.editmode.get())
    keybind_edit()
    #edit_full()
    #load_editfile()
    #rC(".toolbar.program_run","configure","-state","normal")
    #rC(".toolbar.program_step","configure","-state","normal")
    #rC('.menu','entryconfig','Setup','-state','disabled')
    root_window.unbind('<Down>')
    root_window.unbind('<Up>')
    root_window.unbind('<KP_Down>')
    root_window.unbind('<KP_Up>')
    root_window.bind('<Down>', select_next_line)
    root_window.bind('<Up>', select_prev_line)


def auto_tab_lower():
    #save_editfile()
    root_window.unbind('<Down>')
    root_window.unbind('<Up>')
    keybind_edit_restore()
    bind_axis('Down', 'Up', 1)
    bind_axis("KP_Down", "KP_Up", 1)
    #pVars.editsize.set(False)
    #edit_lower()
    #pVars.editmode.set(False)
    #print(pVars.editmode.get())
    #rC(".toolbar.program_run","configure","-state","disabled")
    #rC(".toolbar.program_step","configure","-state","disabled")
    #rC('.menu','entryconfig','Setup','-state','normal')

def tab_auto():
    if s.task_mode == 2:
      #rC('.pane.top.tabs','itemconfigure','manual','-state','disabled')
      #rC('.pane.top.tabs','itemconfigure','mdi','-state','disabled')
      rC('.pane.top.tabs','itemconfigure','edit','-state','disabled')
      #rC('.pane.top.tabs','itemconfigure','edit')
      #rC('.menu','entryconfig','Setup','-state','disabled')
      rC('.pane.top.tabs','raise','auto')
    else:
      #rC('.pane.top.tabs','itemconfigure','manual','-state','normal')
      #rC('.pane.top.tabs','itemconfigure','mdi','-state','normal')
      rC('.pane.top.tabs','itemconfigure','edit','-state','normal')
      #rC('.menu','entryconfig','Setup','-state','normal')

############################################
#######      MOVE THE GCODE   ##############
############################################

# remove bottom pane
rC('.pane','forget','.pane.bottom')
# new auto tab with gcode text
rC('.pane.top.tabs','insert','end','auto','-text',' Auto ','-raisecmd','auto_tab_raise','-leavecmd','auto_tab_lower')
rC('.pane.top.tabs.fauto','configure','-borderwidth',2)
rC('frame','.pane.top.tabs.fauto.t','-borderwidth',2,'-relief','sunken','-highlightthickness','1')
rC('text','.pane.top.tabs.fauto.t.text','-borderwidth','0','-exportselection','0','-highlightthickness','0','-relief','flat','-takefocus','0','-undo','0','-height','30','-wrap','word')
rC('bind','.pane.top.tabs.fauto.t.text','<Configure>','goto_sensible_line')
rC('scrollbar','.pane.top.tabs.fauto.t.sb','-width',25,'-borderwidth','0','-highlightthickness','0')
rC('.pane.top.tabs.fauto.t.text','configure','-state','normal','-yscrollcommand',['.pane.top.tabs.fauto.t.sb','set'])
rC('.pane.top.tabs.fauto.t.sb','configure','-command',['.pane.top.tabs.fauto.t.text','yview'])
rC('pack','.pane.top.tabs.fauto.t.sb','-fill','y','-side','left')
rC('pack','.pane.top.tabs.fauto.t.text','-expand','1','-fill','both','-side','top')
rC('pack','.pane.top.tabs.fauto.t','-fill','both')

# create a new widget list so we can "move" the gcode text
widget_list_new = []
for widget in widget_list:
    if '.t.text' in widget[2]:
        widget = ('text', Text, '.pane.top.tabs.fauto.t.text')
    widget_list_new.append(widget)
    widget_list_new.append(('edit', Text, '.pane.top.tabs.fedit.t.text'))
    widget_list_new.append(('buttonFrame', Frame, '.fbuttons'))
widgets = nf.Widgets(root_window,*widget_list_new)

# copied from axis.py (line 3857) to assign the "new" widgets.text to t
t = widgets.text
t.bind('<Button-3>', rClicker)
t.tag_configure("ignored", background="#ffffff", foreground="#808080")
t.tag_configure("lineno", foreground="#808080")
t.tag_configure("executing", background="#804040", foreground="#ffffff")
t.bind("<Button-1>", select_line)
t.bind("<Double-Button-1>", release_select_line)
t.bind("<B1-Motion>", lambda e: "break")
t.bind("<B1-Leave>", lambda e: "break")
t.bind("<Button-4>", scroll_up)
t.bind("<Button-5>", scroll_down)
t.configure(state="disabled")
######################################
##########   Addition to GCODE #######
######################################
rC('labelframe','.pane.top.tabs.fauto.program')
rC('pack','.pane.top.tabs.fauto.program','-before','.pane.top.tabs.fauto.t','-fill','x')
rC('label','.pane.top.tabs.fauto.program.name','-text','file','-justify','left','-padx',16,'-anchor','ne')
rC('label','.pane.top.tabs.fauto.program.time','-textvariable','runJ','-justify','right','-padx',16)
rC('checkbutton','.pane.top.tabs.fauto.program.size','-text',' AUTO ','-command','edit_size','-variable','editsize')
rC('pack','.pane.top.tabs.fauto.program.size','-side','left')
rC('pack','.pane.top.tabs.fauto.program.name','-side','left')
rC('pack','.pane.top.tabs.fauto.program.time','-side','right')

######################################
#######  end Addition to GCODE #######
######################################

############################################
#######  MOVE THE GCODE/new key binds ######
############################################
def select_next_line(self):
    if o.highlight_line is None:
        i = 1
    else:
        i = max(o.last_line, o.highlight_line + 1)
    o.set_highlight_line(i)
    o.tkRedraw()
##############################################
#####   and do select_prev also #############
########################################
def select_prev_line(self):
    if o.highlight_line is None:
        i = o.last_line
    else:
        i = max(1, o.highlight_line - 1)
    o.set_highlight_line(i)
    o.tkRedraw()

############################################
############################################
#######  end MOVE THE GCODE   ##############
############################################

#########
#########################################################################
#######   EDIT TAB   ######
#########
##################################

edittext = ('.pane.top.tabs.fedit.t.text')

def edit_size():
    rC('grid','propagate','.pane.top.tabs',0)

    if pVars.editsize.get()==True:
        rC('grid','remove','.pane.top.right')
        if pVars.winSize.get() == 'medium':
            rC('grid','.pane.top.tabs','-sticky','nesw','-columnspan',2,'-rowspan',1)
        else:
            rC('grid','.pane.top.tabs','-sticky','nesw','-columnspan',2)
            #rC('grid','columnconfigure',ftop,0,'-weight',0,'-minsize',tabSizeW)
    else:
        if pVars.winSize.get() == 'medium':
            rC('grid','.pane.top.tabs','-sticky','nesw','-columnspan',1,'-rowspan',15)
        else:
            rC('grid','.pane.top.tabs','-sticky','nesw','-columnspan',1)
        rC('grid','.pane.top.right','-sticky','nesw','-columnspan',1)

def edit_full():
    rC('grid','remove','.pane.top.right')
    rC('grid','.pane.top.tabs','-sticky','nesw','-columnspan',2)

def edit_lower():
    rC('grid','.pane.top.tabs','-sticky','nesw','-columnspan',1)
    rC('grid','.pane.top.right','-sticky','nesw','-columnspan',1)

def keybind_edit():
    ### UNBIND
    root_window.unbind("l")#, commands.toggle_override_limits)
    root_window.unbind("o")#, commands.open_file)
    root_window.unbind("s")#, commands.task_resume)
    root_window.unbind("t")#, commands.task_step)
    root_window.unbind("p")#, commands.task_pause)
    root_window.unbind("R")#, commands.task_reverse)
    root_window.unbind("F")#, commands.task_forward)
    root_window.unbind("v")#, commands.cycle_view)
    root_window.unbind("r")#, commands.task_run)
    root_window.unbind("B")#, commands.brake_on)
    root_window.unbind("b")#, commands.brake_off)
    root_window.unbind("x")#, lambda event: activate_ja_widget("x"))
    root_window.unbind("y")#, lambda event: activate_ja_widget("y"))
    root_window.unbind("z")#, lambda event: activate_ja_widget("z"))
    root_window.unbind("a")#, lambda event: activate_ja_widget("a"))
    root_window.unbind("c")#, lambda event: jogspeed_continuous())
    root_window.unbind("d")#, lambda event: widgets.rotate.invoke())
    root_window.unbind("i")#, lambda event: jogspeed_incremental())
    root_window.unbind("I")#, lambda event: jogspeed_incremental(-1))
    root_window.unbind("`")#, lambda event: activate_ja_widget_or_set_feedrate(0))
    root_window.unbind("1")#, lambda event: activate_ja_widget_or_set_feedrate(1))
    root_window.unbind("2")#, lambda event: activate_ja_widget_or_set_feedrate(2))
    root_window.unbind("3")#, lambda event: activate_ja_widget_or_set_feedrate(3))
    root_window.unbind("4")#, lambda event: activate_ja_widget_or_set_feedrate(4))
    root_window.unbind("5")#, lambda event: activate_ja_widget_or_set_feedrate(5))
    root_window.unbind("6")#, lambda event: activate_ja_widget_or_set_feedrate(6))
    root_window.unbind("7")#, lambda event: activate_ja_widget_or_set_feedrate(7))
    root_window.unbind("8")#, lambda event: activate_ja_widget_or_set_feedrate(8))
    root_window.unbind("9")#, lambda event: activate_ja_widget_or_set_feedrate(9))
    root_window.unbind("0")#, lambda event: activate_ja_widget_or_set_feedrate(10))
    root_window.unbind("!")#, "set metric [expr {!$metric}]; redraw")
    root_window.unbind("@")#, commands.toggle_display_type)
    root_window.unbind("#")#, commands.toggle_coord_type)
    root_window.unbind("$")#, commands.toggle_teleop_mode)
    root_window.unbind("<Home>")#, commands.home_joint)
    root_window.unbind("<End>")#, commands.touch_off_system)
    root_window.unbind(".")#, commands.toggle_coord_type)
    root_window.unbind(",")#, commands.toggle_teleop_mode)
    root_window.unbind(";")#, commands.toggle_coord_type)
    root_window.unbind("'")#, commands.toggle_teleop_mode)
        
def keybind_edit_restore():
    ########bind
    root_window.bind("l", commands.toggle_override_limits)
    root_window.bind("o", commands.open_file)
    root_window.bind("s", commands.task_resume)
    root_window.bind("t", commands.task_step)
    root_window.bind("p", commands.task_pause)
    root_window.bind("R", commands.task_reverse)
    root_window.bind("F", commands.task_forward)
    root_window.bind("v", commands.cycle_view)
    root_window.bind("r", commands.task_run)
    root_window.bind("B", commands.brake_on)
    root_window.bind("b", commands.brake_off)
    root_window.bind("x", lambda event: activate_ja_widget("x"))
    root_window.bind("y", lambda event: activate_ja_widget("y"))
    root_window.bind("z", lambda event: activate_ja_widget("z"))
    root_window.bind("a", lambda event: activate_ja_widget("a"))
    root_window.bind("c", lambda event: jogspeed_continuous())
    root_window.bind("d", lambda event: widgets.rotate.invoke())
    root_window.bind("i", lambda event: jogspeed_incremental())
    root_window.bind("I", lambda event: jogspeed_incremental(-1))
    root_window.bind("`", lambda event: activate_ja_widget_or_set_feedrate(0))
    root_window.bind("1", lambda event: activate_ja_widget_or_set_feedrate(1))
    root_window.bind("2", lambda event: activate_ja_widget_or_set_feedrate(2))
    root_window.bind("3", lambda event: activate_ja_widget_or_set_feedrate(3))
    root_window.bind("4", lambda event: activate_ja_widget_or_set_feedrate(4))
    root_window.bind("5", lambda event: activate_ja_widget_or_set_feedrate(5))
    root_window.bind("6", lambda event: activate_ja_widget_or_set_feedrate(6))
    root_window.bind("7", lambda event: activate_ja_widget_or_set_feedrate(7))
    root_window.bind("8", lambda event: activate_ja_widget_or_set_feedrate(8))
    root_window.bind("9", lambda event: activate_ja_widget_or_set_feedrate(9))
    root_window.bind("0", lambda event: activate_ja_widget_or_set_feedrate(10))
    root_window.bind("!", "set metric [expr {!$metric}]; redraw")
    root_window.bind("@", commands.toggle_display_type)
    root_window.bind("#", commands.toggle_coord_type)
    root_window.bind("$", commands.toggle_teleop_mode)
    root_window.bind("<Home>", commands.home_joint)
    root_window.bind("<End>", commands.touch_off_system)
    #root_window.bind('<1>', select_next_line)
    #root_window.bind('<2>', select_prev_line)

def load_editfile():
    print (s.file)
    open_filea = open(s.file,'r')
    rC('.pane.top.tabs.fedit.t.text','delete','1.0','end')
    rC('.pane.top.tabs.fedit.t.text','insert','end',open_filea.read())
    pVars.editfile.set(s.file)
    #print(pVars.editfile.get())
    rC('.pane.top.tabs.fedit.t.text','edit','reset')

def save_editfile():
    print (s.file)
    open_filea = open(s.file,'w')
    edit_gcode = rC('.pane.top.tabs.fedit.t.text','get','1.0','end-1c')
    open_filea.write(edit_gcode)
    reload_file()  
    
    return 1

def edit_tab_raise():
    pVars.editmode.set(True)
    #print(pVars.editmode.get())
    keybind_edit()
    #edit_full()
    load_editfile()
    rC(".toolbar.program_run","configure","-state","disabled")
    rC(".toolbar.program_step","configure","-state","disabled")
    
def edit_tab_lower():
    save_editfile()
    keybind_edit_restore()
    pVars.editsize.set(False)
    edit_lower()
    pVars.editmode.set(False)
    #print(pVars.editmode.get())
    rC(".toolbar.program_run","configure","-state","normal")
    rC(".toolbar.program_step","configure","-state","normal")

def print_file():
    print ('breakin')
    rC('.pane.top.tabs.fedit.t.text','insert','insert', '%K')
    return "break"

# new edit tab with gcode text
rC('.pane.top.tabs','insert','end','edit','-text',' edit ','-raisecmd','edit_tab_raise','-leavecmd','edit_tab_lower')
rC('.pane.top.tabs.fedit','configure','-borderwidth',2)
rC('frame','.pane.top.tabs.fedit.t','-borderwidth',2,'-relief','sunken','-highlightthickness','1')
rC('text','.pane.top.tabs.fedit.t.text','-borderwidth','0','-relief','flat','-takefocus','1','-undo','1','-height','30','-wrap','word')

#rC('bind','.pane.top.tabs.fedit.t.text','<Configure>','goto_sensible_line')
rC('scrollbar','.pane.top.tabs.fedit.t.sb','-width',25,'-borderwidth','0','-highlightthickness','0')
rC('.pane.top.tabs.fedit.t.text','configure','-yscrollcommand',['.pane.top.tabs.fedit.t.sb','set'])
rC('.pane.top.tabs.fedit.t.sb','configure','-command',['.pane.top.tabs.fedit.t.text','yview'])
rC('pack','.pane.top.tabs.fedit.t.sb','-fill','y','-side','left')
rC('pack','.pane.top.tabs.fedit.t.text','-expand','1','-fill','both','-side','right')
rC('pack','.pane.top.tabs.fedit.t','-fill','both')

rC('button','.pane.top.tabs.fedit.edit','-text','edit','-command','load_editfile')
rC('button','.pane.top.tabs.fedit.save','-text','save','-command','save_editfile')
#rC('pack','.pane.top.tabs.fedit.edit','-side','left')
#rC('pack','.pane.top.tabs.fedit.save','-side','left')
######################################
##########   Addition to EDIT #######
######################################
rC('labelframe','.pane.top.tabs.fedit.program')
rC('pack','.pane.top.tabs.fedit.program','-before','.pane.top.tabs.fedit.t','-fill','x')
rC('label','.pane.top.tabs.fedit.program.name','-text','file','-justify','left','-padx',16)
rC('checkbutton','.pane.top.tabs.fedit.program.size','-text',' EDIT ','-command','edit_size','-variable','editsize')
rC('pack','.pane.top.tabs.fedit.program.size','-side','left')
rC('pack','.pane.top.tabs.fedit.program.name','-side','left')
######################################
#######  end Addition to EDIT #######
######################################

#########
##################################
#######   end EDIT TAB   ######
#########
##################################


##### probe   #@######



# new probe tab in top.tabs
rC('.pane.top.tabs','insert','end','probe','-text',' PROBE ')
rC('.pane.top.tabs.fprobe','configure','-borderwidth',2)

# pagesmanager for probe pages
rC('PagesManager','.pane.top.tabs.fprobe.pages')
for probepage in ['bore','boss','web','slot','edge','z','setting']:
    rC('.pane.top.tabs.fprobe.pages','add',probepage)



# buttons to control pages
rC('frame','.pane.top.tabs.fprobe.buttons')
# ~ for n in range(0,6):
    # ~ rC('button','.pane.top.tabs.fprobe.buttons.' + str(n) , '-text','ass' + str(n))
    # ~ rC('pack','.pane.top.tabs.fprobe.buttons.' + str(n),'-fill','both','-side','left','-expand',1)
    
for probebutt in ['bore','boss','web','slot','edge','z','setting']:
    #rC('button','.pane.top.tabs.fprobe.buttons.' + probebutt , '-text','ass' + probebutt)
    rC('button','.pane.top.tabs.fprobe.buttons.' + probebutt , '-text',probebutt)
    rC('pack','.pane.top.tabs.fprobe.buttons.' + probebutt,'-fill','both','-side','left','-expand',1)

#  setting page
rC('frame','.pane.top.tabs.fprobe.pages.fsetting.axis','-borderwidth',2,'-relief','sunken','-highlightthickness','1')



v = StringVar()
v.set("x")
rC('radiobutton','.pane.top.tabs.fprobe.pages.fsetting.axis.edgex','-text','+X','-variable',v,'-value','x','-indicatoron','false')
rC('radiobutton','.pane.top.tabs.fprobe.pages.fsetting.axis.edgenx','-text','-X','-variable',v,'-value','-x','-indicatoron','false')
rC('radiobutton','.pane.top.tabs.fprobe.pages.fsetting.axis.edgey','-text','+Y','-variable',v,'-value','y','-indicatoron','false')
rC('radiobutton','.pane.top.tabs.fprobe.pages.fsetting.axis.edgeny','-text','-Y','-variable',v,'-value','-y','-indicatoron','false')
rC('label','.pane.top.tabs.fprobe.pages.fsetting.axis.edgel','-text','EDGE')

rC('pack','.pane.top.tabs.fprobe.pages.fsetting.axis.edgel','-side','left')
rC('pack','.pane.top.tabs.fprobe.pages.fsetting.axis.edgeny','-side','right')
rC('pack','.pane.top.tabs.fprobe.pages.fsetting.axis.edgey','-side','right')
rC('pack','.pane.top.tabs.fprobe.pages.fsetting.axis.edgenx','-side','right')
rC('pack','.pane.top.tabs.fprobe.pages.fsetting.axis.edgex','-side','right')
rC('pack','.pane.top.tabs.fprobe.pages.fsetting.axis','-fill','both')
rC('pack','.pane.top.tabs.fprobe.buttons','-fill','x','-expand',1,'-side','bottom')
rC('pack','.pane.top.tabs.fprobe.pages','-fill','both','-side','top')

rC('.pane.top.tabs.fprobe.pages','raise','setting')

###### end probe   #####









##############################################################################
# MONKEYPATCHED FUNCTIONS                         # PLASMAC2                #
##############################################################################


def get_coordinate_font(large):         # PLASMAC2
    global coordinate_font
    global coordinate_linespace
    global coordinate_charwidth
    global fontbase
    #coordinate_font = 'monospace {}'.format(fontSize)
    coordinate_font = ngcFont
    if coordinate_font not in font_cache:
        font_cache[coordinate_font] = \
            glnav.use_pango_font(coordinate_font, 0, 128)
    fontbase, coordinate_charwidth, coordinate_linespace = \
            font_cache[coordinate_font]



##############################################################################
# USER BUTTON                                                                #
##############################################################################

def set_toggle_pins(pin):           # PLASMAC2
    pin['state'] = hal.get_value(pin['pin'])
    if pin['state']:
        rC('.fbuttons.button' + pin['button'],'configure','-bg',colorActive)
    else:
         rC('.fbuttons.button' + pin['button'],'configure','-bg',colorBack)
        #if pin['runcritical']:
        #    rC('.fbuttons.button' + pin['button'],'configure','-bg',colorWarn)
        #else:
        #    rC('.fbuttons.button' + pin['button'],'configure','-bg',colorBack)
##############################################################################
# USER BUTTON FUNCTIONS                                                      #
##############################################################################
def validate_hal_pin(halpin, button, usage):            # PLASMAC2
    title = _('HAL PIN ERROR')
    valid = pBit = False
    for pin in halPinList:
        if halpin in pin['NAME']:
            pBit = isinstance(pin['VALUE'], bool)
            valid = True
            break
    if not valid:
        msg0 = _('does not exist for user button')
        notifications.add('error', '{}:\n{} {} #{}'.format(title, halpin, msg0, button))
    if not pBit:
        msg0 = _('must be a bit pin for user button')
        notifications.add('error', '{}:\n{} {} #{}'.format(title, usage, msg0, button))
        valid = False
    return valid

def validate_ini_param(code, button):           # plasmac
    title = _('PARAMETER ERROR')
    valid = [False, None]
    try:
        parm = code[code.index('{') + len('') + 1: code.index('}')]
        value = inifile.find(parm.split()[0], parm.split()[1]) or None
        if value:
            valid = [True, code.replace('{{{}}}'.format(parm), value)]
    except:
        pass
    if not valid[0]:
        msg0 = _('invalid parameter')
        msg1 = _('for user button')
        notifications.add('error', '{}:\n{} {} {} #{}'.format(title, msg0, code, msg1, button))
    return valid

def button_action(button, pressed):         # plasmac
    if int(pressed):
        user_button_pressed(button, buttonCodes[int(button)])
    else:
        user_button_released(button, buttonCodes[int(button)])

def user_button_setup():        # plasmac
    global buttonNames, buttonCodes, togglePins, criticalButtons#, fontSize#, pulsePins, machineBounds, criticalButtons
#    global probeButton, probeText, torchButton, torchText, cChangeButton
    singleCodes = []#'ohmic-test','cut-type','single-cut','manual-cut','probe-test', \
#                   'torch-pulse','change-consumables','framing','latest-file']
    buttonNames = {0:{'name':None}}
    buttonCodes = {0:{'code':None}}
    criticalButtons = []
    row = 1
    for n in range(1,20):
        bLabel = None
        bName = getPrefs(PREF,'BUTTONS', str(n) + ' Name', '', str)
        bCode = getPrefs(PREF,'BUTTONS', str(n) + ' Code', '', str)
        outCode = {'code':None}
        # if bCode.strip() == 'ohmic-test' and not 'ohmic-test' in [(v['code']) for k, v in buttonCodes.items()]:
            # outCode['code'] = 'ohmic-test'
        # elif bCode.strip() == 'cut-type' and not 'cut-type' in buttonCodes:
            # bName = bName.split(',')
            # if len(bName) == 1:
                # text = _('Pierce\Only') if '\\' in bName[0] else _('Pierce Only')
                # bName.append(text)
            # outCode = {'code':'cut-type', 'text':bName}
        # elif bCode.strip() == 'single-cut' and not 'single-cut' in buttonCodes:
            # outCode['code'] = 'single-cut'
        # elif bCode.strip() == 'manual-cut' and not 'manual-cut' in buttonCodes:
            # outCode['code'] = 'manual-cut'
        # elif bCode.startswith('probe-test') and not 'probe-test' in [(v['code']) for k, v in buttonCodes.items()]:
            # if bCode.split()[0].strip() == 'probe-test' and len(bCode.split()) < 3:
                # codes = bCode.strip().split()
                # outCode = {'code':'probe-test', 'time':10}
                # probeButton = str(n)
                # probeText = bName.replace('\\', '\n')
                # if len(codes) == 2:
                    # try:
                        # value = int(float(codes[1]))
                        # outCode['time'] = value
                    # except:
                        # outCode['code'] = None
        # elif bCode.startswith('torch-pulse') and not 'torch-pulse' in [(v['code']) for k, v in buttonCodes.items()]:
            # if bCode.split()[0].strip() == 'torch-pulse' and len(bCode.split()) < 3:
                # codes = bCode.strip().split()
                # outCode = {'code':'torch-pulse', 'time':1.0}
                # torchButton = str(n)
                # torchText = bName.replace('\\', '\n')
                # if len(codes) == 2:
                    # try:
                        # value = round(float(codes[1]), 1)
                        # outCode['time'] = value
                    # except:
                        # outCode['code'] = None
        # elif bCode.startswith('change-consumables ') and not 'change-consumables' in [(v['code']) for k, v in buttonCodes.items()]:
            # codes = re.sub(r'([xyf]|[XYF])\s+', r'\1', bCode) # remove any spaces after x, y, and f
            # codes = codes.lower().strip().split()
            # if len(codes) > 1 and len(codes) < 5:
                # outCode = {'code':'change-consumables', 'X':None, 'Y':None, 'F':None}
                # for l in 'xyf':
                    # for c in range(1, len(codes)):
                        # if codes[c].startswith(l):
                            # try:
                                # value = round(float(codes[c].replace(l,'')), 3)
                                # outCode['XYF'['xyf'.index(l)]] = value
                            # except:
                                # outCode['code'] = None
                # if (not outCode['X'] and not outCode['Y']) or not outCode['F']:
                    # outCode['code'] = None
                # else:
                    # buff = 10 * hal.get_value('halui.machine.units-per-mm') # keep 10mm away from machine limits
                    # for axis in 'XY':
                        # if outCode[axis]:
                            # if outCode['{}'.format(axis)] < machineBounds['{}-'.format(axis)] + buff:
                                # outCode['{}'.format(axis)] = machineBounds['{}-'.format(axis)] + buff
                            # elif outCode['{}'.format(axis)] > machineBounds['{}+'.format(axis)] - buff:
                                # outCode['{}'.format(axis)] = machineBounds['{}+'.format(axis)] - buff
            # if outCode['code']:
                # cChangeButton = str(n)
        # elif bCode.startswith('framing') and not 'framing' in [(v['code']) for k, v in buttonCodes.items()]:
            # codes = re.sub(r'([f]|[F])\s+', r'\1', bCode) # remove any spaces after f
            # codes = codes.lower().strip().split()
            # if codes[0] == 'framing' and len(codes) < 4:
                # outCode = {'code':'framing', 'F':False, 'Z':False}
                # for c in range(1, len(codes)):
                    # if codes[c].startswith('f'):
                        # try:
                            # value = round(float(codes[c].replace('f','')), 3)
                            # outCode['F'] = value
                        # except:
                            # outCode['code'] = None
                    # elif codes[c] == 'usecurrentzheight':
                        # outCode['Z'] = True
                    # else:
                        # outCode['code'] = None
        # elif bCode.startswith('load '):
            # if len(bCode.split()) > 1 and len(bCode.split()) < 3:
                # codes = bCode.strip().split()
                # if os.path.isfile(os.path.join(open_directory, codes[1])):
                    # outCode = {'code':'load', 'file':os.path.join(open_directory, codes[1])}
        # elif bCode.startswith('latest-file') and not 'latest-file' in [(v['code']) for k, v in buttonCodes.items()]:
            # if len(bCode.split()) < 3:
                # codes = bCode.strip().split()
                # outCode = {'code':'latest-file', 'dir':None}
                # if len(codes) == 1:
                    # outCode['dir'] = open_directory
                # elif len(codes) == 2 and os.path.isdir(codes[1]):
                    # outCode['dir'] = codes[1]
                # else:
                    # outCode['code'] = None
        # elif bCode.startswith('pulse-halpin '):
            # if len(bCode.split()) > 1 and len(bCode.split()) < 4:
                # codes = bCode.strip().split()
                # if validate_hal_pin(codes[1], n, 'pulse-halpin'):
                    # outCode = {'code':'pulse-halpin', 'pin':codes[1], 'time':1.0}
                    # outCode['pin'] = codes[1]
                    # try:
                        # value = round(float(codes[2]), 1)
                        # outCode['time'] = value
                        # pulsePins[str(n)] = {'button':str(n), 'pin':outCode['pin'], 'text':None, 'timer':0, 'counter':0, 'state':False}
                    # except:
                        # outCode = {'code':None}
        if bCode.startswith('toggle-halpin '):
            if len(bCode.split()) > 1 and len(bCode.split()) < 4:
                codes = bCode.strip().split()
                if validate_hal_pin(codes[1], n, 'toggle-halpin'):
                    outCode = {'code':'toggle-halpin', 'pin':codes[1], 'critical':False}
                    outCode['pin'] = codes[1]
                    if len(codes) == 3 and codes[2] == 'runcritical':
                        outCode['critical'] = True
                        criticalButtons.append(n)
                    togglePins[str(n)] = {'button':str(n), 'pin':outCode['pin'], 'state':hal.get_value(outCode['pin']), 'runcritical':outCode['critical']}
        elif bCode and bCode not in singleCodes:
            codes = bCode.strip().split('\\')
            codes = [x.strip() for x in codes]
            outCode['code'] = []
            for cn in range(len(codes)):
                if codes[cn][0] == '%':
                    if WHICH(codes[cn].split()[0][1:]) is not None:
                        outCode['code'].append(['shell', codes[cn][1:]])
                    else:
                        outCode = {'code': None}
                elif codes[cn][:2].lower() == 'o<':
                    outCode['code'].append(['ocode', codes[cn]])
                elif codes[cn][0].lower() in 'gm':
                    if not '{' in codes[cn]:
                        outCode['code'].append(['gcode', codes[cn]])
                    else:
                        reply = validate_ini_param(codes[cn], n)
                        if reply[0]:
                            outCode['code'].append(['gcode', reply[1]])
                        else:
                            outCode = {'code': None}
                            break
                else:
                    outCode = {'code': None}
                    break
        else:
            outCode = {'code':None}
        if not rC('winfo','exists','.fbuttons.button' + str(n)):
            ubuttSize = int( int(fontSize) - 2)
            #print(ubuttSize)
            rC('button','.fbuttons.button' + str(n),'-takefocus',0,'-width',ubuttSize)
        if bName and outCode['code']:
            bHeight = 2
            if type(bName) == list:
                bHeight = max(len(bName[0].split('\\')), len(bName[1].split('\\')))
                bName = bName[0]
            else:
                bHeight = len(bName.split('\\')) 
            bLabel = bName.replace('\\', '\n')
            ubuttSize = int( int(fontSize) - 2)
            rC('.fbuttons.button' + str(n),'configure','-text',bLabel,'-height',bHeight,'-bg',colorBack,'-wraplength',60,'-width',ubuttSize)
            #print(ubuttSize)
             # change to pack 
            #rC('grid','.fbuttons.button{}'.format(n),'-column',row,'-row',0,'-sticky','nsew','-padx',(2,0),'-pady',(2,0))
            rC('pack','.fbuttons.button{}'.format(n),'-side','left','-fill','both','-expand',1)
            rC('bind','.fbuttons.button{}'.format(n),'<ButtonPress-1>','button_action {} 1'.format(n))
            rC('bind','.fbuttons.button{}'.format(n),'<ButtonRelease-1>','button_action {} 0'.format(n))
            row += 1
        elif bName or bCode:
            title = _('USER BUTTON ERROR')
            msg0 = _('is invalid code for user button')
            notifications.add('error', '{}:\n"{}" {} #{}'.format(title, bCode, msg0, n))
            bName = None
            outCode = {'code':None}
        #print (bHeight)
        buttonNames[n] = {'name':bName}
        buttonCodes[n] = outCode
    user_button_load()

def user_button_pressed(button, code):      # plasmac
    global colorBack, activeFunction
#    global probePressed, probeStart, probeTimer, probeButton
#    global torchPressed, torchStart, torchTimer, torchButton

    if rC('.fbuttons.button' + button,'cget','-state') == 'disabled' or not code:
        return
    from subprocess import Popen,PIPE
    # ~ if code['code'] == 'ohmic-test':
        # ~ hal.set_p('plasmac.ohmic-test','1')
# ~ #FIXME: TEMPORARY PRINT FOR REPORTING WINDOW SIZES
        # ~ print('Width={}   Height={}'.format(rC('winfo','width',root_window), rC('winfo','height',root_window)))
    # ~ elif code['code'] == 'cut-type':
        # ~ pass # actioned from button_release
    # ~ elif code['code'] == 'single-cut':
        # ~ pass # actioned from button_release
    # ~ elif code['code'] == 'manual-cut':
        # ~ manual_cut(None)
    # ~ elif code['code'] == 'probe-test' and not hal.get_value('halui.program.is-running'):
        # ~ if probeTimer:
            # ~ probeTimer = 0
        # ~ elif not hal.get_value('plasmac.z-offset-counts'):
            # ~ activeFunction = True
            # ~ probePressed = True
            # ~ probeStart = time.time()
            # ~ probeTimer = code['time']
            # ~ hal.set_p('plasmac.probe-test','1')
            # ~ rC('.fbuttons.button' + probeButton,'configure','-text',str(int(probeTimer)))
            # ~ rC('.fbuttons.button' + probeButton,'configure','-bg',colorActive)
    # ~ elif code['code'] == 'torch-pulse':
        # ~ if torchTimer:
            # ~ torchTimer = 0
        # ~ elif not hal.get_value('plasmac.z-offset-counts'):
            # ~ torchPressed = True
            # ~ torchStart = time.time()
            # ~ torchTimer = code['time']
            # ~ hal.set_p('plasmac.torch-pulse-time','{}'.format(torchTimer))
            # ~ hal.set_p('plasmac.torch-pulse-start','1')
            # ~ rC('.fbuttons.button' + torchButton,'configure','-text',str(int(torchTimer)))
            # ~ rC('.fbuttons.button' + torchButton,'configure','-bg',colorActive)
    # ~ elif code['code'] == 'change-consumables' and not hal.get_value('plasmac.breakaway'):
        # ~ if hal.get_value('axis.x.eoffset-counts') or hal.get_value('axis.y.eoffset-counts'):
            # ~ hal.set_p('plasmac.consumable-change', '0')
            # ~ hal.set_p('plasmac.x-offset', '0')
            # ~ hal.set_p('plasmac.y-offset', '0')
            # ~ rC('.fbuttons.button' + button,'configure','-bg',colorBack)
            # ~ activeFunction = False
        # ~ else:
            # ~ activeFunction = True
            # ~ xPos = s.position[0] if code['X'] is None else code['X']
            # ~ yPos = s.position[1] if code['Y'] is None else code['Y']
            # ~ hal.set_p('plasmac.xy-feed-rate', str(code['F']))
            # ~ hal.set_p('plasmac.x-offset', '{:.0f}'.format((xPos - s.position[0]) / hal.get_value('plasmac.offset-scale')))
            # ~ hal.set_p('plasmac.y-offset', '{:.0f}'.format((yPos - s.position[1]) / hal.get_value('plasmac.offset-scale')))
            # ~ hal.set_p('plasmac.consumable-change', '1')
            # ~ rC('.fbuttons.button' + button,'configure','-bg',colorOrange)
    # ~ elif code['code'] == 'framing':
        # ~ pass # actioned from button_release
    # ~ elif code['code'] == 'load':
        # ~ pass # actioned from button_release
    # ~ elif code['code'] == 'latest-file':
        # ~ pass # actioned from button_release
    # ~ elif code['code'] == 'pulse-halpin' and hal.get_value('halui.program.is-idle'):
        # ~ hal.set_p(code['pin'], str(not hal.get_value(code['pin'])))
        # ~ if not pulsePins[button]['timer']:
            # ~ pulsePins[button]['text'] = rC('.fbuttons.button' + button,'cget','-text')
            # ~ pulsePins[button]['timer'] = code['time']
            # ~ pulsePins[button]['counter'] = time.time()
        # ~ else:
            # ~ pulsePins[button]['timer'] = 0
            # ~ rC('.fbuttons.button' + button,'configure','-text',pulsePins[button]['text'])
    if code['code'] == 'toggle-halpin' and hal.get_value('halui.program.is-idle'):
        hal.set_p(code['pin'], str(not hal.get_value(code['pin'])))
    else:
        for n in range(len(code['code'])):
            if code['code'][n][0] == 'shell':
                Popen(code['code'][n][1], stdout=PIPE, stderr=PIPE, shell=True)
            elif code['code'][n][0] in ['gcode', 'ocode']:
                if manual_ok():
                    ensure_mode(linuxcnc.MODE_MDI)
                    commands.send_mdi_command(code['code'][n][1])

def user_button_released(button, code):         # plasmac
#    global cutType, probePressed, torchPressed
    if rC('.fbuttons.button' + button,'cget','-state') == 'disabled' or not code: return
#    if code['code'] == 'ohmic-test':
#        hal.set_p('plasmac.ohmic-test','0')
#    elif code['code'] == 'cut-type':
#        if not hal.get_value('halui.program.is-running'):
#            cutType ^= 1
#            if cutType:
#                comp['cut-type'] = 1
#                text = code['text'][1].replace('\\', '\n')
#                color = colorOrange
#            else:
#                comp['cut-type'] = 0
#                text = code['text'][0].replace('\\', '\n')
#                color = colorBack
#            rC('.fbuttons.button' + button,'configure','-bg',color,'-text',text)
#            reload_file()
#    elif code['code'] == 'single-cut':
#        single_cut()
#    elif code['code'] == 'manual-cut':
#        pass
#    elif code['code'] == 'probe-test':
#        probePressed = False
#    elif code['code'] == 'torch-pulse':
#        torchPressed = False
#    elif code['code'] == 'change-consumables':
#        pass
#    elif code['code'] == 'framing':
#        if not code['F']:
#            code['F'] = int(rC('.runs.material.cut-feed-rate', 'get'))
#        frame_job(code['F'], code['Z'])
#    elif code['code'] == 'load':
#        commands.open_file_name(code['file'])
#    elif code['code'] == 'latest-file':
#        files = GLOB('{}/*.ngc'.format(code['dir']))
#        latest = max(files, key=os.path.getctime)
#        commands.open_file_name(latest)
#    elif code['code'] == 'pulse-halpin':
#        pass
#    elif code['code'] == 'toggle-halpin':
 #       pass
    else:
        pass

def user_button_add():          # plasmac
    for n in range(1, 20):
        if not rC('winfo','ismapped',fsetup + '.tabs.fbutt.r.ubuttons.frame.num' + str(n)):
            rC('grid',fsetup + '.tabs.fbutt.r.ubuttons.frame.num' + str(n),'-column',0,'-row',n,'-sticky','ne','-padx',(4,0),'-pady',(0,4))
            rC('grid',fsetup + '.tabs.fbutt.r.ubuttons.frame.name' + str(n),'-column',1,'-row',n,'-sticky','nw','-padx',(4,0),'-pady',(0,4))
            rC('grid',fsetup + '.tabs.fbutt.r.ubuttons.frame.code' + str(n),'-column',2,'-row',n,'-sticky','new','-padx',(4,4),'-pady',(0,4))
            

            break
        ##cbbox = rC(fsetup + '.tabs.fbutt.r.ubuttons.frame','bbox',"all")
        ##rC(fsetup + '.tabs.fbutt.r.ubuttons.frame','configure','-scrollregion',(cbbox))
    #cbbox = rC(fsetup + '.tabs.fbutt.r.ubuttons.frame','bbox',("ALL"))
    #rC(fsetup + '.tabs.fbutt.r.ubuttons.frame','configure','-scrollregion',(0,0,50,500))
    ###rC(fsetup + '.tabs.fbutt.r.ubuttons.frame','yview','moveto',1.0)
    #print(rC(fsetup + '.tabs.fbutt.r.ubuttons.frame','bbox',("all")))
    #print(rC('winfo','children', fsetup + '.tabs.fbutt.r.ubuttons.frame'))

def user_button_load():         # plasmac
    hide_buttonframe()
    #rC(fsetup + '.tabs.fbutt.r.torch.enabled','delete',0,'end')
    #rC(fsetup + '.tabs.fbutt.r.torch.disabled','delete',0,'end')
    #rC(fsetup + '.tabs.fbutt.r.torch.enabled','insert','end',getPrefs(PREF,'BUTTONS', 'Torch enabled', 'Torch\Enabled', str))
    #rC(fsetup + '.tabs.fbutt.r.torch.disabled','insert','end',getPrefs(PREF,'BUTTONS','Torch disabled', 'Torch\Disabled', str))
    for n in range(1, 20):
        rC('grid','forget',fsetup + '.tabs.fbutt.r.ubuttons.frame.num' + str(n))
        rC('grid','forget',fsetup + '.tabs.fbutt.r.ubuttons.frame.name' + str(n))
        rC('grid','forget',fsetup + '.tabs.fbutt.r.ubuttons.frame.code' + str(n))
        rC(fsetup + '.tabs.fbutt.r.ubuttons.frame.name' + str(n),'delete',0,'end')
        rC(fsetup + '.tabs.fbutt.r.ubuttons.frame.code' + str(n),'delete',0,'end')
        if getPrefs(PREF,'BUTTONS', str(n) + ' Name', '', str) or getPrefs(PREF,'BUTTONS', str(n) + ' Code', '', str):
            rC(fsetup + '.tabs.fbutt.r.ubuttons.frame.name' + str(n),'insert','end',getPrefs(PREF,'BUTTONS', str(n) + ' Name', '', str))
            rC(fsetup + '.tabs.fbutt.r.ubuttons.frame.code' + str(n),'insert','end',getPrefs(PREF,'BUTTONS', str(n) + ' Code', '', str))
            rC('grid',fsetup + '.tabs.fbutt.r.ubuttons.frame.num' + str(n),'-column',0,'-row',n,'-sticky','ne','-padx',(4,0),'-pady',(0,4))
            rC('grid',fsetup + '.tabs.fbutt.r.ubuttons.frame.name' + str(n),'-column',1,'-row',n,'-sticky','nw','-padx',(4,0),'-pady',(0,4))
            rC('grid',fsetup + '.tabs.fbutt.r.ubuttons.frame.code' + str(n),'-column',2,'-row',n,'-sticky','new','-padx',(4,4),'-pady',(0,4))
            color_user_buttons()
        #rC(fsetup + '.tabs.fbutt.r.ubuttons.frame','create','window',0,0,'-anchor','nw','-window',fsetup + '.tabs.fbutt.r.ubuttons.frame')
    ##cbbox = rC(fsetup + '.tabs.fbutt.r.ubuttons.frame','bbox',"all")
    ##rC(fsetup + '.tabs.fbutt.r.ubuttons.frame','configure','-scrollregion',(cbbox))
    hide_buttonframe()
    #print(rC(fsetup + '.tabs.fbutt.r.ubuttons.frame','bbox',("all")))
    #print(rC('winfo','children', fsetup + '.tabs.fbutt.r.ubuttons.frame'))
def user_button_save():
#    global torchEnable
#    putPrefs(PREF,'BUTTONS', 'Torch enabled', rC(fsetup + '.tabs.fbutt.r.torch.enabled','get'), str)
#    putPrefs(PREF,'BUTTONS', 'Torch disabled', rC(fsetup + '.tabs.fbutt.r.torch.disabled','get'), str)
#    torchEnable['enabled'] = getPrefs(PREF,'BUTTONS', 'Torch enabled', 'Torch\Enabled', str)
#    torchEnable['disabled'] = getPrefs(PREF,'BUTTONS','Torch disabled', 'Torch\Disabled', str)
#    if '\\' in torchEnable['enabled'] or '\\' in torchEnable['disabled']:
#        rC('.fbuttons.torch-enable','configure','-height',2)
#    else:
#        rC('.fbuttons.torch-enable','configure','-height',1)
#    color_torch()
    for n in range(1, 20):
        putPrefs(PREF,'BUTTONS', '{} Name'.format(n), rC(fsetup + '.tabs.fbutt.r.ubuttons.frame.name' + str(n),'get'), str)
        putPrefs(PREF,'BUTTONS', '{} Code'.format(n), rC(fsetup + '.tabs.fbutt.r.ubuttons.frame.code' + str(n),'get'), str)
        rC('grid','forget',fsetup + '.tabs.fbutt.r.ubuttons.frame.num' + str(n))
        rC('grid','forget',fsetup + '.tabs.fbutt.r.ubuttons.frame.name' + str(n))
        rC('grid','forget',fsetup + '.tabs.fbutt.r.ubuttons.frame.code' + str(n))
        #rC('grid','forget','.fbuttons.button' + str(n))
        #### change to pack
        rC('pack','forget','.fbuttons.button' + str(n))
        rC(fsetup + '.tabs.fbutt.r.ubuttons.frame.name' + str(n),'delete',0,'end')
        rC(fsetup + '.tabs.fbutt.r.ubuttons.frame.code' + str(n),'delete',0,'end')
    user_button_setup()


######################################################################
#########3      screen layout stuff     Z###############################
##################################################################

def set_screensmall():
  #print("butt")
  #rC('grid','forget','.info')
  ##rC(".pane","configure","-height","300",'-width','700')
  rC(".pane","configure","-height","300")
  ##rC('.pane.top.tabs','configure','-width','325')
  rC('.pane.top.right','configure','-width','300')
  ##rC('.pane.top.right','configure','-width','350')
  #rC("wm","geometry",".","+0+36")
  rC(fjogf + '.zerohome.tooltouch','configure','-text','Tool Touch')
  rC('grid',fjogf + '.zerohome.home','-rowspan',3,'-padx',3)
  rC(fjogf + '.zerohome.home','configure','-height',3,'-padx',3)
  rC('grid','forget','.pane.top.gcodel')
  rC('grid','forget','.pane.top.right')
  rC('grid','forget','.pane.top.tabs')
  rC('grid','forget','.pane.top.gcodes')
  rC('grid','forget','.pane.top.feedoverride')
  rC('grid','forget','.pane.top.rapidoverride')
  rC('grid','forget','.pane.top.spinoverride')
  rC('grid','forget','.pane.top.jogspeed')
  rC('grid','forget','.pane.top.ajogspeed')
  rC('grid','forget','.pane.top.maxvel') 
  rC('grid','.pane.top.tabs','-sticky','nesw','-column','0','-row','1','-rowspan',1)
  rC('grid','.pane.top.spinoverride','-sticky','new')
  rC('grid','.pane.top.feedoverride','-sticky','new')
  rC('grid','.pane.top.right','-sticky','nesw','-column','1','-row','1','-rowspan',1)
  #rC('grid','rowconfigure','.pane.top.right',1,'-weight',3)

  rC('.pane.top.gcodes','configure','-height',3,'-width',20)
  rC('grid','.pane.top.gcodes','-sticky','nesw','-column','1','-row','2','-rowspan',3)
  #rC('grid','.pane.top.gcodes','-sticky','nesw','-column','1')
  rC(ftabs,'configure','-homogeneous',False)
  rC(fright,'configure','-homogeneous',False)
  #rC('wm','minsize','.','750','200')

def screen_medium():
  #rC(".pane","configure","-height","500",'-width','950');
  #rC('.pane.top.tabs','configure','-width','500')
  rC('grid','forget','.pane.top.tabs')
  rC('grid','forget','.pane.top.feedoverride')
  rC('grid','forget','.pane.top.rapidoverride')
  rC('grid','forget','.pane.top.spinoverride')
  rC('grid','forget','.pane.top.jogspeed')
  rC('grid','forget','.pane.top.ajogspeed')
  rC('grid','forget','.pane.top.maxvel')
  rC('grid','remove','.pane.top.gcodel')
  rC('grid','forget','.pane.top.gcodes') 
  rC('grid','.pane.top.right','-column','1','-sticky','nesw')
  #rC('grid','.pane.top.gcodes','-column','1','-sticky','new')
  rC('grid','.pane.top.right','-sticky','nesw','-column','1','-row','1','-rowspan',1)
  rC('grid','.pane.top.feedoverride','-sticky','nesw','-column','1')
  rC('grid','.pane.top.spinoverride','-sticky','nesw','-column','1')
  rC('grid','.pane.top.gcodes','-sticky','nesw','-column','1','-rowspan',1)
  rC('.pane.top.gcodes','configure','-height',3)
  rC('grid','.pane.top.tabs','-sticky','nesw','-column','0','-row','1','-rowspan',10)
  rC(ftabs,'configure','-homogeneous',True)
  rC(fright,'configure','-homogeneous',True)

def toolbar_config():
  rC('pack','forget','.toolbar.rule0')
  rC('pack','forget','.toolbar.rule4')
  rC('pack','forget','.toolbar.rule8')
  rC('pack','forget','.toolbar.rule9')
  rC('pack','forget','.toolbar.rule12')
  for w in toolButtons:
                rC('pack','forget','.toolbar.{}'.format(w))
  bSize = int(int(fontSize) / 10 * 24) if int(fontSize) > 10 else 24
  buttFont = '{} {}'.format(fontName, str(int(fontSize) - 2))
  #print(bSize)
  if pVars.buttlab.get() == True:       
          for w in toolButtons:
                rC('.toolbar.{}'.format(w),'configure' ,"-bd",1,"-font",buttFont
                ,"-compound","top",'-width',(bSize - 4),'-height',(bSize + 8))
          rC(".toolbar.machine_estop","configure","-text","ESTOP","-activebackground","tomato2")
          rC(".toolbar.machine_power","configure","-text","POWER","-activebackground","palegreen3")
          rC(".toolbar.file_open","configure","-text","OPEN","-activebackground","grey90")
          rC(".toolbar.reload","configure","-text","RELOAD","-activebackground","palegreen1")
          rC(".toolbar.program_run","configure","-text","RUN")
          rC(".toolbar.program_step","configure","-text","STEP")
          rC(".toolbar.program_pause","configure","-text","PAUSE")
          rC(".toolbar.program_stop","configure","-text","STOP")
          rC(".toolbar.program_blockdelete","configure","-text","SKIP")
          rC(".toolbar.program_optpause","configure","-text","M1")
          rC(".toolbar.view_zoomin","configure","-text","ZOOM")
          rC(".toolbar.view_zoomout","configure","-text","ZOOM")
          rC(".toolbar.view_z","configure","-text","VIEW")
          rC('pack','forget','.toolbar.view_z2')
          rC('pack','forget','.toolbar.view_y2')
          rC(".toolbar.view_x","configure","-text","VIEW")
          rC(".toolbar.view_y","configure","-text","VIEW")
          rC(".toolbar.view_p","configure","-text","VIEW")
          rC(".toolbar.rotate","configure","-text","ROTATE")
          rC(".toolbar.clear_plot","configure","-text","CLEAR")
          for w in toolButtons:
             rC('pack','.toolbar.{}'.format(w),'-fill','x','-side','left','-expand',1)
          rC('pack','forget','.toolbar.view_z2')
          rC('pack','forget','.toolbar.view_y2')
          rC('pack','.toolbar.rule0','-after','.toolbar.machine_power','-fill','x','-side','left','-expand',1)
          rC('pack','.toolbar.rule4','-after','.toolbar.reload','-fill','x','-side','left','-expand',1)
          rC('pack','.toolbar.rule8','-after','.toolbar.program_stop','-fill','x','-side','left','-expand',1)
          rC('pack','.toolbar.rule9','-after','.toolbar.program_optpause','-fill','x','-side','left','-expand',1)
          rC('pack','.toolbar.rule12','-after','.toolbar.rotate','-fill','x','-side','left','-expand',1)
  else:
       # no button labels
       for w in toolButtons:
                rC('.toolbar.{}'.format(w),'configure',"-compound","none","-text",""
                ,"-activebackground",colorFore,'-width',bSize,'-height',bSize,'-bd',1)
       for w in toolButtons:
             rC('pack','.toolbar.{}'.format(w),'-fill','x','-side','left','-expand',1)
       rC('pack','.toolbar.rule0','-after','.toolbar.machine_power','-fill','x','-side','left','-expand',1)
       rC('pack','.toolbar.rule4','-after','.toolbar.reload','-fill','x','-side','left','-expand',1)
       rC('pack','.toolbar.rule8','-after','.toolbar.program_stop','-fill','x','-side','left','-expand',1)
       rC('pack','.toolbar.rule9','-after','.toolbar.program_optpause','-fill','x','-side','left','-expand',1)
       rC('pack','.toolbar.rule12','-after','.toolbar.rotate','-fill','x','-side','left','-expand',1)
  rC('pack','forget','.toolbar.view_z2')
  rC('pack','forget','.toolbar.view_y2')
  
def hide_buttonframe():
    if pVars.hidebuttf.get() == True:
        rC('grid','forget','.fbuttons')
    else:
        rC('grid','.fbuttons','-column',0,'-row',3,'-rowspan',2,'-sticky','nsew','-padx',0,'-pady',0)

##########################################################################
####     SLIDER STUFF      ######################################################
##########################################################################

def hide_jog():
    if pVars.hidejog.get() == True:
        rC('grid','forget',ftop + '.jogspeed')
        rC('grid','forget',ftop + '.ajogspeed')
    else:
      if pVars.winSize.get() == 'medium':
        rC('grid','forget','.pane.top.gcodes') 
        rC('grid','forget','.pane.top.tabs')
        rC('grid','.pane.top.jogspeed','-column',1,'-sticky','new')        
        # test if we need the angular jog slider, 56==0x38==000111000==ABC
        #if s.axis_mask & 56 == 0 and 'ANGULAR' in joint_type:
        if has_angular_joint_or_axis:
            rC('grid','.pane.top.ajogspeed','-column',1,'-sticky','new')
            print("angular slider")
        rC('grid','.pane.top.gcodes','-sticky','nesw','-column','1','-rowspan',1)
        rC('grid','.pane.top.tabs','-sticky','nesw','-column','0','-row','1','-rowspan',15)
      elif  pVars.winSize.get() == 'small':
        rC('grid','.pane.top.jogspeed','-column',0,'-sticky','new')
        # test if we need the angular jog slider, 56==0x38==000111000==ABC
        #if s.axis_mask & 56 == 0 and 'ANGULAR' in joint_type:
        if has_angular_joint_or_axis:
            rC('grid','.pane.top.ajogspeed','-column',0,'-sticky','new')
      else:
        rC('grid','forget','.pane.top.gcodes') 
        rC('grid','forget','.pane.top.gcodel') 
        rC('grid','forget','.pane.top.right')
        rC('grid','.pane.top.jogspeed','-column',0,'-sticky','new')
        # test if we need the angular jog slider, 56==0x38==000111000==ABC
        #if s.axis_mask & 56 == 0 and 'ANGULAR' in joint_type:
        if has_angular_joint_or_axis:
            rC('grid','.pane.top.ajogspeed','-column',0,'-sticky','new')
            print("angular slider")
        rC('grid','.pane.top.gcodel','-sticky','nw','-column',0,'-rowspan',1)
        rC('grid','.pane.top.gcodes','-sticky','nesw','-column',0,'-rowspan',1)
        rC('grid','.pane.top.right','-sticky','nesw','-column','1','-row','1','-rowspan',15)
  
def hide_rapid():
    if pVars.hiderapid.get() == True:
        rC('grid','forget',ftop + '.rapidoverride')
    else:
      if pVars.winSize.get() == 'medium':
        rC('grid','forget','.pane.top.gcodes') 
        rC('grid','forget','.pane.top.tabs')
        rC('grid','.pane.top.rapidoverride','-column',1,'-sticky','new')
        rC('grid','.pane.top.gcodes','-sticky','nesw','-column','1','-rowspan',1)
        rC('grid','.pane.top.tabs','-sticky','nesw','-column','0','-row','1','-rowspan',15)
      elif  pVars.winSize.get() == 'small':
        rC('grid','.pane.top.rapidoverride','-column',0,'-sticky','new')
      else:
        rC('grid','forget','.pane.top.gcodes') 
        rC('grid','forget','.pane.top.gcodel') 
        rC('grid','forget','.pane.top.right')
        rC('grid','.pane.top.rapidoverride','-column',0,'-sticky','new')
        rC('grid','.pane.top.gcodel','-sticky','nw','-column',0,'-rowspan',1)
        rC('grid','.pane.top.gcodes','-sticky','nesw','-column',0,'-rowspan',1)
        rC('grid','.pane.top.right','-sticky','nesw','-column','1','-row','1','-rowspan',15)
         
def hide_velocity():
    
    if pVars.hidevel.get() == True:
        rC('grid','forget',ftop + '.maxvel',)
    else:
      if pVars.winSize.get() == 'medium':
        rC('grid','forget','.pane.top.gcodes') 
        rC('grid','forget','.pane.top.tabs')
        rC('grid','.pane.top.maxvel','-column',1,'-sticky','new')
        rC('grid','.pane.top.gcodes','-sticky','nesw','-column','1','-rowspan',1)
        rC('grid','.pane.top.tabs','-sticky','nesw','-column','0','-row','1','-rowspan',15)
      elif  pVars.winSize.get() == 'small':
            rC('grid','.pane.top.maxvel','-column',0,'-sticky','new')
      else:
        rC('grid','forget','.pane.top.gcodes') 
        rC('grid','forget','.pane.top.gcodel') 
        rC('grid','forget','.pane.top.right')
        rC('grid','.pane.top.maxvel','-column',0,'-sticky','new')
        rC('grid','.pane.top.gcodel','-sticky','nw','-column',0,'-rowspan',1)
        rC('grid','.pane.top.gcodes','-sticky','nesw','-column',0,'-rowspan',1)
        rC('grid','.pane.top.right','-sticky','nesw','-column','1','-row','1','-rowspan',15)


##########################################################################
####   end  SLIDER STUFF      ######################################################
##########################################################################

###   pop up dialog things

def show_dialog(func, title, msg):      # plasmac
    dlg = plasmacDialog(func, title, msg)
    root_window.wait_window(dlg.dlg)
    return(dlg.reply)

def prompt_touchoff(title, text, default, tool_only, system=None):          # plasmac
    title = _('TOUCH OFF')
    text = text.replace(':', '') % _('workpiece')
    dlg = plasmacDialog('touch', title, text, system)
    root_window.wait_window(dlg.dlg)
    valid, value, system = dlg.reply
    try:
        v = float(value)
    except:
        msg0 = _('Touch off entry {} is invalid'.format(value))
        show_dialog('error', title, msg0)
        value = 0.0
    if valid:
        return(value, system)
    else:
        return(None, None)
        
def prompt_areyousure(title, text):                     #plasmac
    #title = _('TOUCH OFF')
    #text = text.replace(':', '') % _('workpiece')
    dlg = plasmacDialog('yesno', title, text)
    root_window.wait_window(dlg.dlg)
    #valid, value, system = dlg.reply
    #return t.run()
    value = dlg.reply
    return value

def font_size_changed():                            # plasmac modified
    global fontName, fontSize, startFontSize, ngcFont
    #fontName = 'sans'
    fontName = 'mono'
    fontSize = pVars.fontSize.get()
    font = '{} {}'.format(fontName, fontSize)
    arcFont = '{} {}'.format(fontName, str(int(fontSize) * 3))
    ngcFont = '{} {}'.format(fontName, str(int(fontSize) - 1))
    rC('font','configure','TkDefaultFont','-family', fontName, '-size', fontSize)
#    rC(fplasma + '.arc-voltage','configure','-font',arcFont)
#   rC('.pane.bottom.t.text','configure','-height',8,'-font',font)
    rC('.pane.top.tabs.fauto.t.text','configure','-font',ngcFont)
    rC('.pane.top.tabs.fedit.t.text','configure','-font',ngcFont)
    #print(arcFont)
    #print(ngcFont)    
    rC('.pane.top.gcodes','configure','-font',ngcFont)
    #rC(ftabs,'delete','manual',0)
    #rC(ftabs,'delete','mdi',0)
    #rC(ftabs,'insert','end','manual')
    #rC(ftabs,'insert','end','mdi')
    #rC(ftabs,'raise','manual')
    #rC(ftabs,'itemconfigure','manual','-text','MANUAL')
    #rC(ftabs,'itemconfigure','mdi','-text','MDI')
    #rC(fright,'delete','preview',0)
    #rC(fright,'delete','numbers',0)
    #rC(fright,'delete','stats',0)
    #rC(fright,'insert','end','preview')
    #rC(fright,'insert','end','numbers')
    #rC(fright,'insert','end','stats')
    #rC(fright,'raise','preview')
    #rC(fright,'itemconfigure','preview','-text',_('PREVIEW'))
    #rC(fright,'itemconfigure','numbers','-text',_('DRO'))
    #rC(fright,'itemconfigure','stats','-text',_('Statistics'))
    rC('.pane.top.tabs','itemconfigure','manual','-text',' MANUAL')
    rC('.pane.top.tabs','itemconfigure','mdi','-text',' MDI')
    rC('.pane.top.tabs','itemconfigure','auto','-text',' AUTO')
    rC('.pane.top.tabs','itemconfigure','edit','-text',' EDIT ')
    rC('.pane.top.right','itemconfigure','preview','-text',' PREVIEW ')
    rC('.pane.top.right','itemconfigure','numbers','-text',' DRO ')
    
    rC(fsetup + '.tabs','itemconfigure','gui','-text','  GUI  ')
    rC(fsetup + '.tabs','itemconfigure','butt','-text','  BUTTONS  ')
    pagest = (rC(fsetup + '.tabs','pages'))
    for page in pagest:
        rC(fsetup + '.tabs','itemconfigure',page,'-foreground',colorFore,'-background',colorBack,'-activeforeground',colorBack,'-activebackground',colorFore)
 
    for nbook in [ftabs, fright]:
        pages = (rC(nbook,'pages'))
        for page in pages:
            rC(nbook,'itemconfigure',page,'-foreground',colorFore,'-background',colorBack)
    rC(fright + '.fpreview','configure','-bd',0)
    rC(fright + '.fnumbers','configure','-bd',0)
    #rC(fright + '.fstats','configure','-bd',2)
    for box in spinBoxes:
        rC(box,'configure','-font',fontName + ' ' + fontSize)
    #ledFrame = int(fontSize) * 2
    #ledSize = ledFrame - 2
    #ledScale = int(fontSize) / int(startFontSize)
    startFontSize = fontSize
    #for led in wLeds:
     #   rC(led,'configure','-width',ledFrame,'-height',ledFrame)
      #  rC(led,'scale',1,1,1,ledScale,ledScale)
    bSize = int(int(fontSize) / 10 * 24) if int(fontSize) > 10 else 24
    #print(bSize)
    for w in toolButtons:
        rC('.toolbar.{}'.format(w),'configure','-width',bSize,'-height',bSize)
    #for w in matButtons:
     #   rC('.toolmat.{}'.format(w),'configure','-width',bSize,'-height',bSize)
    toolbar_config()
    user_button_setup()
    rC('grid','forget','.fbuttons')
    get_coordinate_font(None)
    rC('update','idletasks')
    

def set_window_size():       # plasmac modified
    global tabSizeW
    size = pVars.winSize.get()
    if size not in ['default', 'last', 'fullscreen', 'maximized','small','medium']:
        title = _('WINDOW ERROR')
        msg0 = _('Invalid parameters in [GUI_OPTIONS]Window size in preferences file')
        notifications.add('error', '{}:\n{}\n'.format(title, msg0))
        return False
    else:
        if size == 'maximized':
            rC('wm','attributes','.','-fullscreen', 0)
            rC('wm','attributes','.','-zoomed',1)
        elif size == 'small':
            set_screensmall()
            width = 750
            height = 400
            tabSizeW = int((width / 2)-20)
            rC('wm','geometry','.','{}x{}'.format(width, height))
            rC('.pane.top.right','configure','-width',280)
            rC('.pane.top.tabs','configure','-width',460)
            rC(".pane","configure","-height","300")
            #rC('grid','columnconfigure',ftop,0,'-weight',0,'-minsize',tabSizeW)
            #rC('tk::PlaceWindow','.')
            
        elif size == 'medium':
            screen_medium()
            #rC('wm','attributes','.','-zoomed', 0)
            #rC('wm','attributes','.','-fullscreen', 0)
            width = 1000
            height = 570
            tabSizeW = int((width / 1.6)-20)
            rC('wm','geometry','.','{}x{}'.format(width, height))
            
            rC('.pane.top.tabs','configure','-width',tabSizeW)
            rC('grid','columnconfigure',ftop,0,'-weight',0,'-minsize',tabSizeW)
            #rC('tk::PlaceWindow','.')           
            
        elif size == 'fullscreen':
            rC('wm','attributes','.','-zoomed', 0)
            rC('wm','attributes','.','-fullscreen',1)
        elif size == 'last':
            size = getPrefs(PREF,'GUI_OPTIONS', 'Window last', 'none', str)
            if size == 'none':
                size = 'default'
            else:
                rC('wm','attributes','.','-zoomed', 0)
                rC('wm','attributes','.','-fullscreen', 0)
                rC('wm','geometry','.',size)
        if size == 'default':
            rC('grid','.pane.top.feedoverride','-sticky','nesw','-column','0')
            rC('grid','.pane.top.spinoverride','-sticky','nesw','-column','0')
            rC('wm','attributes','.','-zoomed', 0)
            rC('wm','attributes','.','-fullscreen', 0)
            width = 900
            height = 600
            tabSizeW = int((width / 2)-20)
            rC('wm','geometry','.','{}x{}'.format(width, height))
            #rC('.pane.top.tabs','configure','-width',tabSizeW)
            rC('.pane.top.tabs','configure','-width',600)
            #rC('.pane.top.right','configure','-width',300)
            rC('grid','.pane.top.tabs','-sticky','nesw','-columnspan',1,'-rowspan',1)

            #rC('grid','columnconfigure',ftop,0,'-minsize',tabSizeW)
            #print (tabSizeW)
            #rC('tk::PlaceWindow','.')
    #hide_buttonframe()
    rC(ftabs,'configure','-homogeneous',True,'-tabpady','1''1')
    rC(fright,'configure','-homogeneous',True,'-tabpady','1''1')
    #rC('.pane.top.tabs','configure','-width',500)
    #rC('.pane.top.right','configure','-width',300)
    #rC(".pane","configure","-height","300")
    #rC('grid','columnconfigure',ftop,0,'-weight',0,'-minsize',tabSizeW)
    #rC('grid','.pane.top.tabs','-sticky','nesw','-columnspan',1,'-rowspan',1)
    hide_rapid()
    hide_jog()
    hide_velocity()
    set_window_screen()
    
def set_window_screen():
    size = pVars.winScreen.get()
    if size not in ['default', 'last', 'fullscreen', 'maximized']:
        title = _('WINDOW ERROR')
        msg0 = _('Invalid parameters in [GUI_OPTIONS]Window size in preferences file')
        notifications.add('error', '{}:\n{}\n'.format(title, msg0))
        return False
    else:
        if size == 'maximized':
            rC('wm','attributes','.','-fullscreen', 0)
            rC('wm','attributes','.','-zoomed',1)

        elif size == 'fullscreen':
            rC('wm','attributes','.','-zoomed', 0)
            rC('wm','attributes','.','-fullscreen',1)
        elif size == 'last':
            size = getPrefs(PREF,'GUI_OPTIONS', 'Window last', 'none', str)
            if size == 'none':
                size = 'default'
            else:
                rC('wm','attributes','.','-zoomed', 0)
                rC('wm','attributes','.','-fullscreen', 0)
                rC('wm','geometry','.',size)
        if size == 'default':
            rC('wm','attributes','.','-zoomed', 0)
            rC('wm','attributes','.','-fullscreen', 0)
            #width = 750
            #height = 400
            #rC('wm','geometry','.','{}x{}'.format(width, height))

            rC('tk::PlaceWindow','.')

def load_setup_clicked():                    # plasmac
    #rC(fsetup + '.tabs.fgui.l.jog.speed','set',restoreSetup['jogSpeed'])
    #jog_default_changed(restoreSetup['jogSpeed'])
    if pVars.winSize.get() != restoreSetup['winSize']:
        pVars.winSize.set(restoreSetup['winSize'])
        set_window_size()
    if pVars.winScreen.get() != restoreSetup['winScreen']:
        pVars.winScreen.set(restoreSetup['winScreen'])
        set_window_screen()        
    if pVars.fontSize.get() != restoreSetup['fontSize']:
        pVars.fontSize.set(restoreSetup['fontSize'])
        font_size_changed()
    if pVars.fontSize.get() != restoreSetup['fontSize']:
        pVars.fontSize.set(restoreSetup['fontSize'])
        font_size_changed()
    #if pVars.kbShortcuts.get() != restoreSetup['kbShortcuts']:
    #    pVars.kbShortcuts.set(restoreSetup['kbShortcuts'])
    #    keyboard_bindings(restoreSetup['kbShortcuts'])
    pVars.closeDialog.set(restoreSetup['closeDialog'])
    #rC(fsetup + '.tabs.fgui.l.gui.zoom','set',restoreSetup['tableZoom'])
    if pVars.hidejog.get() != restoreSetup['hidejog']:
        pVars.hidejog.set(restoreSetup['hidejog'])
        hide_jog()
    if pVars.hiderapid.get() != restoreSetup['hiderapid']:
        pVars.hiderapid.set(restoreSetup['hiderapid'])
        hide_rapid()
    if pVars.hidevel.get() != restoreSetup['hidevel']:
        pVars.hidevel.set(restoreSetup['hidevel'])
        hide_velocity()
    if pVars.buttlab.get() != restoreSetup['buttlab']:
        pVars.buttlab.set(restoreSetup['buttlab'])
        toolbar_config()
    if pVars.load_last.get() != restoreSetup['load_last']:
        pVars.loadlast.set(restoreSetup['load_last'])
    if pVars.hidebuttf.get() != restoreSetup['hidebuttf']:
        pVars.hidebuttf.set(restoreSetup['hidebuttf'])
        hide_buttonframe()        
    user_button_load()
    read_colors()
    color_change()
    hide_jog()
    hide_rapid()
    hide_velocity()    
    hide_buttonframe()
    rC('grid','forget','.fbuttons')    
    toolbar_config()

def save_setup_clicked():                    # plasmac 
    #if int(rC(fsetup + '.tabs.fgui.l.jog.speed','get')) < minJogSpeed:
     #   rC(fsetup + '.tabs.fgui.l.jog.speed','set',minJogSpeed)
    #restoreSetup['jogSpeed'] = rC(fsetup + '.tabs.fgui.l.jog.speed','get')
    #putPrefs(PREF,'GUI_OPTIONS', 'Jog speed', restoreSetup['jogSpeed'], int)
    restoreSetup['closeDialog'] = pVars.closeDialog.get()
    putPrefs(PREF,'GUI_OPTIONS', 'Exit warning', restoreSetup['closeDialog'], bool)
    restoreSetup['winSize'] = pVars.winSize.get()
    putPrefs(PREF,'GUI_OPTIONS', 'Window size', restoreSetup['winSize'], str)
    restoreSetup['winScreen'] = pVars.winScreen.get()
    putPrefs(PREF,'GUI_OPTIONS', 'Window screen', restoreSetup['winScreen'], str)
    restoreSetup['fontSize'] = pVars.fontSize.get()
    putPrefs(PREF,'GUI_OPTIONS', 'Font size', restoreSetup['fontSize'], str)
    #restoreSetup['kbShortcuts'] = pVars.kbShortcuts.get()
    #putPrefs(PREF,'GUI_OPTIONS', 'Use keyboard shortcuts', restoreSetup['kbShortcuts'], bool)
    restoreSetup['hidejog'] = pVars.hidejog.get()
    putPrefs(PREF,'GUI_OPTIONS', 'Hide Max Jog. Slider', restoreSetup['hidejog'], bool)
    restoreSetup['hiderapid'] = pVars.hiderapid.get()
    putPrefs(PREF,'GUI_OPTIONS', 'Hide Max Rapid. Slider', restoreSetup['hiderapid'], bool)
    restoreSetup['hidevel'] = pVars.hidevel.get()
    putPrefs(PREF,'GUI_OPTIONS', 'Hide Max Velocity Slider', restoreSetup['hidevel'], bool)
    restoreSetup['buttlab'] = pVars.buttlab.get()
    putPrefs(PREF,'GUI_OPTIONS', 'Show Button Label', restoreSetup['buttlab'], bool)
    restoreSetup['load_last'] = pVars.load_last.get()
    putPrefs(PREF,'GUI_OPTIONS', 'Load last file', restoreSetup['load_last'], bool)
    user_button_save()
    restoreSetup['hidebuttf'] = pVars.hidebuttf.get()
    putPrefs(PREF,'GUI_OPTIONS', 'Hide Button Frame', restoreSetup['hidebuttf'], bool)
    hide_buttonframe()
    rC('grid','forget','.fbuttons')
    putPrefs(PREF,'GUI_OPTIONS', 'Foreground color', colorFore, str)
    putPrefs(PREF,'GUI_OPTIONS', 'Background color', colorBack, str)
    putPrefs(PREF,'GUI_OPTIONS', 'Disabled color', colorDisable, str)
    putPrefs(PREF,'GUI_OPTIONS', 'Active color', colorActive, str)
    putPrefs(PREF,'GUI_OPTIONS', 'Warning color', colorWarn, str)
    putPrefs(PREF,'GUI_OPTIONS', 'Voltage color', colorVolt, str)
    for key in togglePins:
        set_toggle_pins(togglePins[key]) 



##############################################################################
# EXTERNAL HAL PINS                                                          #
##############################################################################

# called during setup
def ext_hal_create():           # PLASMAC2
    global extHalPins
    extHalPins = {}
    for pin in ['abort', 'power', 'run', 'pause', 'run-pause', 'touchoff', 'probe-test',
                'torch-pulse', 'frame-job']:
        comp.newpin('ext.{}'.format(pin), hal.HAL_BIT, hal.HAL_IN)
        extHalPins[pin] = {'state': False, 'last': False}

# called every cycle by user_live_update
def ext_hal_watch():            # PLASMAC2
    global extHalPins, isIdle, isIdleHomed, isPaused, isRunning, probePressed, torchPressed
    for pin in extHalPins:
        state = comp['ext.{}'.format(pin)]
        if state != extHalPins[pin]['last']:
            extHalPins[pin]['last'] = state
            # pressed commands
            if state:
                if pin == 'abort':
                    commands.task_stop()
                elif pin == 'power':
                    commands.onoff_clicked()
                elif pin == 'run' and isIdleHomed:
                    commands.task_run()
                elif pin == 'pause':
                    if isRunning:
                        commands.task_pause()
                    elif isPaused:
                        commands.task_resume()
                elif pin == 'run-pause':
                    if isIdleHomed:
                        commands.task_run()
                    elif isRunning:
                        commands.task_pause()
                    elif isPaused:
                        commands.task_resume()
                elif pin == 'touchoff':
                    touch_off_xy('1', '0', '0')
                elif pin == 'probe-test' and probeButton:
                    user_button_pressed(probeButton, buttonCodes[int(probeButton)])
                elif pin == 'torch-pulse' and torchButton:
                    user_button_pressed(torchButton, buttonCodes[int(torchButton)])
            # released commands
            else:
                if pin == 'touchoff':
                    touch_off_xy('0', '0', '0')
                elif pin == 'probe-test' and probeButton:
                    user_button_released(probeButton, buttonCodes[int(probeButton)])
                elif pin == 'torch-pulse' and torchButton:
                    user_button_released(torchButton, buttonCodes[int(torchButton)])
                elif pin == 'frame-job':
                    num = get_button_num('framing')
                    if num:
                        user_button_released(str(num), buttonCodes[num])

def get_button_num(name):           # PLASMAC2
    num = None
    for num in buttonCodes:
        if buttonCodes[num]['code'] == name:
            break
    return num


##############################################################################
# END USER BUTTON                                                                #
##############################################################################

##############################################################################
# COLOR CHANGE                                                               #
##############################################################################
def read_colors():              # PLASMAC2
    global colorFore, colorBack, colorDisable, colorActive
    global colorWarn, colorVolt, colorOrange, colorYellow
    colorFore = getPrefs(PREF,'GUI_OPTIONS','Foreground color', '#000000', str)
    colorBack = getPrefs(PREF,'GUI_OPTIONS','Background color', '#d9d9d9', str)
    colorDisable = getPrefs(PREF,'GUI_OPTIONS','Disabled color', '#a3a3a3', str)
    colorActive = getPrefs(PREF,'GUI_OPTIONS','Active color', '#00cc00', str)
    colorWarn = getPrefs(PREF,'GUI_OPTIONS','Warning color', '#dd0000', str)
    colorVolt = getPrefs(PREF,'GUI_OPTIONS','Voltage color', '#0000ff', str)
    colorOrange = '#FFAA00'
    colorYellow = '#FFFF00'

def color_user_buttons(fgc='#000000',bgc='#d9d9d9'):            # PLASMAC2
#    for b in criticalButtons:
#        rC('.fbuttons.button' + str(b),'configure','-bg',colorWarn)
    # user button entries in setup frame
    for n in range(1, 20):
        # ~ rC(fsetup + '.tabs.fbutt.r.ubuttons.frame.name' + str(n),'configure','-fg',colorBack)
        # ~ rC(fsetup + '.tabs.fbutt.r.ubuttons.frame.name' + str(n),'configure','-bg',colorFore)
        # ~ rC(fsetup + '.tabs.fbutt.r.ubuttons.frame.code' + str(n),'configure','-fg',colorBack)
        # ~ rC(fsetup + '.tabs.fbutt.r.ubuttons.frame.code' + str(n),'configure','-bg',colorFore)
        rC(fsetup + '.tabs.fbutt.r.ubuttons.frame.name' + str(n),'configure','-fg',colorFore)
        rC(fsetup + '.tabs.fbutt.r.ubuttons.frame.name' + str(n),'configure','-bg',colorBack)
        rC(fsetup + '.tabs.fbutt.r.ubuttons.frame.code' + str(n),'configure','-fg',colorFore)
        rC(fsetup + '.tabs.fbutt.r.ubuttons.frame.code' + str(n),'configure','-bg',colorBack)
    

        # ~ if buttonNames[n]['name']:
            # ~ rC(fsetup + '.tabs.fbutt.r.ubuttons.frame.name' + str(n),'configure','-fg',colorFore)
            # ~ rC(fsetup + '.tabs.fbutt.r.ubuttons.frame.name' + str(n),'configure','-bg',colorBack)
            # ~ rC(fsetup + '.tabs.fbutt.r.ubuttons.frame.code' + str(n),'configure','-fg',colorFore)
            # ~ rC(fsetup + '.tabs.fbutt.r.ubuttons.frame.code' + str(n),'configure','-bg',colorBack)
        # ~ else:
            # ~ rC(fsetup + '.tabs.fbutt.r.ubuttons.frame.name' + str(n),'configure','-fg',colorBack)
            # ~ rC(fsetup + '.tabs.fbutt.r.ubuttons.frame.name' + str(n),'configure','-bg',colorFore)
            # ~ rC(fsetup + '.tabs.fbutt.r.ubuttons.frame.code' + str(n),'configure','-fg',colorBack)
            # ~ rC(fsetup + '.tabs.fbutt.r.ubuttons.frame.code' + str(n),'configure','-bg',colorFore)

def get_all_children(parent):           # PLASMAC2
    _list = []
    _tup = rC('winfo','children',parent)
    for item in _tup:
        _list.append(item)
    for item in _list:
        if rC('winfo','children',item):
            _tup = (rC('winfo','children',item))
            for item in _tup:
                _list.append(item)
    return _list

def color_change():             # PLASMAC2
    widgetTypes = []
    for child in get_all_children('.'):
        w = rC('winfo','class',child)
        #print(w)
        # root window
        rC('.','configure','-bg',colorBack)
        t.tag_configure("ignored", background="#ffffff", foreground="#808080")
        t.tag_configure("lineno", foreground=colorActive)
        t.tag_configure("executing", background=colorFore, foreground=colorBack)
        # all widgets
        try:
            rC(child,'configure','-fg',colorFore)
        except:
            pass
        try:
#FIXME: I am sitting on the fence with this
#            if w in ['Spinbox'] or (w == 'Entry' and child[:-2] in comboEntries):
#                rC(child,'configure','-bg',ourWhite)
#            else:
#                rC(child,'configure','-bg',bgc)
            rC(child,'configure','-bg',colorBack)
        except:
            pass
        try:
            rC(child,'configure','-disabledforeground',colorDisable)
        except:
            pass
        try:
            rC(child,'configure','-activebackground',colorFore)
        except:
            pass
        try:
            rC(child,'configure','-insertbackground',colorFore)
        except:
            pass
        try:
            rC(child,'configure','-readonlybackground',colorBack)
        except:
            pass
        try:
            rC(child,'configure','-buttonbackground',colorBack)
        except:
            pass
       # try:
       #     rC(child,'configure','-highlightthickness',1)
       # except:
       #     pass
        try:
            rC(child,'configure','-highlightcolor',colorFore)
        except:
            pass
        try:
            rC(child,'configure','-highlightbackground',colorFore)
        except:
            pass
        try:
            rC(child,'configure','-activeforeground',colorBack)
        except:
            pass
        try:
            rC(child,'configure','-selectbackground',colorFore,'-selectforeground',colorBack)
        except:
            pass
        try:
            rC(child,'configure','-relief','flat')
        except:
            pass
        try:
            if w == 'Menu':
                rC(child,'configure','-selectcolor',colorFore)
            else:
                rC(child,'configure','-selectcolor',colorActive)
        except:
            pass
        try:
            # color the trough of override scales and user button scrollbars
            rC(child,'configure','-troughcolor',colorFore,'-activebackground',colorBack)
            # color the trough of combobox lists
            rC('option','add','*Scrollbar.troughColor',colorFore)
            rC('option','add','*Scrollbar.background',colorBack)
            rC('option','add','*Scrollbar.activeBackground',colorBack)
        except:
            pass
        # all comboboxes except for the jog increment Combobox
        
        if w == 'ComboBox':
            rC(child,'configure','-background',colorBack)
            rC(child,'configure','-selectbackground',colorFore)
            rC(child,'configure','-selectforeground',colorBack)
            rC(child,'configure','-relief','flat')
            # lose the arrow
            rC('pack','forget','{}.a'.format(child))
        # the entry of the jog increment combobox
        elif '.jogincr' in child and w == 'Entry': 
            rC(child,'configure','-disabledforeground',colorFore)
        # the listbox of the jog increment combobox
        elif '.jogincr' in child and w == 'Listbox': 
            rC(child,'configure','-selectforeground',colorBack)
            rC(child,'configure','-selectbackground',colorFore)
            rC(child,'configure','-relief','flat')
        # all checkbuttons
        elif w in ['Checkbutton']:
            rC(child,'configure','-relief','flat','-overrelief','raised','-bd',1,'-indicatoron',0)
            #rC(child,'-indicatoron',0)
    # notebook tabs - cutrecs is also done each time it is raised
    for nbook in [ftabs, fright]:
        pages = (rC(nbook,'pages'))
        for page in pages:
            rC(nbook,'itemconfigure',page,'-foreground',colorFore,'-background',colorBack,'-activeforeground',colorBack,'-activebackground',colorFore)
    pagest = (rC(fsetup + '.tabs','pages'))
    for page in pagest:
        rC(fsetup + '.tabs','itemconfigure',page,'-foreground',colorFore,'-background',colorBack,'-activeforeground',colorBack,'-activebackground',colorFore)
    rC(fsetup + '.tabs.fbutt.r.ubuttons','configure','-background',colorBack)
    #rC(fsetup + '.tabs.fbutt.r.ubuttons','_themechanged')
    rC(fsetup + '.tabs.fbutt.r.ubuttons.frame','configure','-bg',colorBack)
    #print(rC(fsetup + '.tabs.fbutt.r.ubuttons.frame','configure'))
    color_user_buttons()
    #color_torch()
#FIXME: I am sitting on the fence with this
    # gcode view
#    rC('.pane.bottom.t.text','configure','-foreground',colorBlue)
#FIXME: I am sitting on the fence with this
    # dro
#    rC('.pane.top.right.fnumbers.text','configure','-foreground',colorActive,'-background',ourBlack)
    # the color setup buttons
    rC(fsetup + '.tabs.fgui.m.colors.fore','configure','-bg',colorFore,'-activebackground',colorFore)
    rC(fsetup + '.tabs.fgui.m.colors.back','configure','-bg',colorBack,'-activebackground',colorBack)
    rC(fsetup + '.tabs.fgui.m.colors.disable','configure','-bg',colorDisable,'-activebackground',colorDisable)
    rC(fsetup + '.tabs.fgui.m.colors.active','configure','-bg',colorActive,'-activebackground',colorActive)
    rC(fsetup + '.tabs.fgui.m.colors.warn','configure','-bg',colorWarn,'-activebackground',colorWarn)
    rC(fsetup + '.tabs.fgui.m.colors.volt','configure','-bg',colorVolt,'-activebackground',colorVolt)
    # notifications
   
    rC('option','add','*!notification2.Frame.Background',colorBack)
    rC('option','add','*!notification2.Frame.Label.Foreground',colorFore)
    rC('option','add','*!notification2.Frame.Label.Background',colorBack)
    rC('option','add','*!notification2.Frame.Button.Background',colorBack)
    rC('option','add','*!notification2.Frame.Button.activeBackground',colorBack)
    rC('option','add','*!notification2.Frame.Button.highlightThickness', 0)

def color_set(option):          # PLASMAC2
    global colorFore, colorBack, colorDisable, colorActive, colorWarn, colorVolt
    if option == 'fore':
        color = colorFore
    elif option == 'back':
        color = colorBack
    elif option == 'disable':
        color = colorDisable
    elif option == 'active':
        color = colorActive
    elif option == 'warn':
       color = colorWarn
    elif option == 'volt':
       color = colorVolt
    else:
        color = '#000000'
    colors = askcolor(color, title=_('plasmac2 Color Selector'))
    if colors[1]:
        if option == 'fore':
            colorFore = colors[1]
        elif option == 'back':
            colorBack = colors[1]
        elif option == 'disable':
            colorDisable = colors[1]
        elif option == 'active':
            colorActive = colors[1]
        elif option == 'warn':
            colorWarn = colors[1]
        elif option == 'volt':
            colorVolt = colors[1]
        color_change()

############################################################
##############   setup  #########'##############################
################### start  #########'
############################################################
def enable_menus(state):            # PLASMAC2
    state = 'normal' if state else 'disabled'
    menus = ['File', 'Machine', 'View','Setup', 'Help']
    for menu in menus:
        rC('.menu','entryconfig',menu,'-state',state)


def setup_toggle(state):            # PLASMAC2
    if int( state):
        hide_default()
        enable_menus(False)
        rC('grid','.toolsetup','-column',0,'-row',0,'-columnspan',3,'-sticky','nesw')
        rC('grid',fsetup,'-column',0,'-row',1,'-rowspan',3,'-sticky','nsew')
        #keyboard_bindings(False)
    else:
        rC('grid','forget','.toolsetup')
        rC('grid','forget',fsetup)
        show_default()


def hide_default():                 # PLASMAC2
    rC('grid','forget','.pane')
    rC('grid','forget','.fbuttons')
    #rC('grid','forget','.fconv')
    rC('grid','forget','.toolbar')
    #rC('grid','forget','.toolmat')
    #rC('grid','forget','.runs')

def show_default():                 # PLASMAC2
    rC('grid','.toolbar','-column',0,'-row',0,'-columnspan',3,'-sticky','nesw')
    rC('grid','.pane','-column',0,'-row',1,'-rowspan',2,'-sticky','nsew')
    #rC('grid','.fbuttons','-column',0,'-row',3,'-rowspan',2,'-sticky','nsew','-padx',0,'-pady',0)
    hide_buttonframe()
    enable_menus(True)
    #keyboard_bindings(pVars.kbShortcuts.get())
    rC('focus','.')

def close_window():             # PLASMAC2
    if pVars.closeDialog.get():
        msgs = ''
        text2 = _('Do you really want to close LinuxCNC ?')
        msgs  += text2
        if not show_dialog('yesno', _('CONFIRM CLOSE'), msgs):
            return
    putPrefs(PREF,'GUI_OPTIONS', 'Window last', rC('winfo','geometry',root_window), str)
    root_window.destroy()
        

###################################
###   timer  ###############
##################################
def secs_to_hms(s):
    m, s = divmod(int(s), 60)
    h, m = divmod(m, 60)
    return '{:02.0f}:{:02.0f}:{:02.0f}'.format(h, m, s)

def hms_to_secs(hms):
    h, m, s = hms.split(':')
    return int(h)*3600 + int(m)*60 + int(s)

def save_total_stats():
    val = hms_to_secs(pVars.runT.get())
    pVars.runS.set(val)
    putPrefs(PREF,'STATISTICS', 'Program run time', val, int)

def clear_job_stats():
    pVars.runJ.set('00:00:00')

def reset_all_stats(stat):
    if stat == 'run':
        pVars.runS.set(0)
        pVars.runJ.set('00:00:00')
        pVars.runT.set('00:00:00')
        putPrefs(PREF,'STATISTICS', 'Program run time', 0, int)



#########################################
#### setup start   ###################
#########################################

from tkinter import messagebox
from tkinter import simpledialog
from tkinter.colorchooser import askcolor
from math import radians, atan, degrees
from collections import OrderedDict
from glob import glob as GLOB
from shutil import copy as COPY
from shutil import which as WHICH
from importlib import reload

# set the default font
#fontName = 'sans'
fontName = 'mono'
fontSize = startFontSize = '10'
# make some widget names to save typing
ftop = '.pane.top'
ftabs = ftop + '.tabs'
fright = ftop + '.right'
fmanual = ftabs + '.fmanual'
faxes = fmanual + '.axes'
fjoints = fmanual + '.joints'
fjogf = fmanual + '.jogf'
foverride = fmanual + '.override'
#flimitovr = fmanual + '.jogf.limitovr'
#fjogiovr = fmanual + '.jogf.inhibitovr'
fmdi = ftabs + '.fmdi'
fsetup = '.setup'
pVars = nf.Variables(root_window,
             
             ('closeDialog', BooleanVar),
             ('kbShortcuts', BooleanVar),
             ('winSize', StringVar),
             ('winScreen', StringVar),
             ('startLine', IntVar),
             ('rflActive', BooleanVar),
             ('preRflFile', StringVar),
             ('jogSpeed', DoubleVar),
             ('fontSize', StringVar),
             ('fontSize', StringVar),
             ('screensmall', StringVar),
             ('hidejog', BooleanVar),
             ('hiderapid', BooleanVar),
             ('hidevel', BooleanVar),
             ('buttlab', BooleanVar),
             ('hidebuttf', BooleanVar),
             ('editmode', BooleanVar),
             ('editsize', BooleanVar),
             ('editfile', StringVar),
             ('load_last', BooleanVar),
             ('runJ', StringVar),
             ('runT', StringVar),
             ('runS', IntVar),

             )
restoreSetup = {}
pVars.fontSize.set(getPrefs(PREF,'GUI_OPTIONS','Font size', '10', str))
#fontSize = pVars.fontSize.get()
restoreSetup['fontSize'] = pVars.fontSize.get()
read_colors()
togglePins = {}
prompt_areyousure = prompt_areyousure
lastMotionMode = None

rC('.menu','insert',4,'command','-command','setup_toggle 1')
#rC('.menu','add','command','-command','setup_toggle 1')
rC('setup_menu_accel','.menu',4,_('_Setup'))

##############################################################################
# GUI ALTERATIONS AND ADDITIONS                                              #
##############################################################################

#######   DOIN THE MANUAL TAB


def ja_button_setup(widget, button, text):
    rC('radiobutton', widget,'-value',button,'-text',text,'-anchor','center', \
       '-variable','ja_rbutton','-command','ja_button_activated','-padx',10,'-pady',10, \
       '-indicatoron',0,'-bd',2,'-highlightthickness',0,'-selectcolor',colorActive)
       
def ja_button_activated():
    if vars.ja_rbutton.get() in 'xyzabcuvw':
        widget = getattr(widgets, 'axis_%s' % vars.ja_rbutton.get())
        widget.focus()
        rC(fjogf + '.zerohome.zero','configure','-text',vars.ja_rbutton.get().upper() + '0')
        #if not homing_order_defined:
         #   widgets.homebutton.configure(text = _('Home') + ' ' + vars.ja_rbutton.get().upper() )
    else:
        widget = getattr(widgets, 'joint_%s' % vars.ja_rbutton.get())
        widget.focus()
    commands.axis_activated()


def joint_mode_switch(a, b, c):
    global lastMotionMode
    if vars.motion_mode.get() == linuxcnc.TRAJ_MODE_FREE and s.kinematics_type != linuxcnc.KINEMATICS_IDENTITY:
        rC('grid','forget',fmanual + '.axes')
        rC('grid',fmanual + '.joints','-column','0','-row','0','-padx','2','-sticky','w')
        widget = getattr(widgets, 'joint_%d' % 0)
        widget.focus()
        vars.ja_rbutton.set(0)
    elif lastMotionMode == linuxcnc.TRAJ_MODE_FREE or not lastMotionMode:
        rC('grid','forget',fmanual + '.joints')
        rC('grid',fmanual + '.axes','-column','0','-row','0','-padx','2','-sticky','w','-columnspan',2)
        widget = getattr(widgets, 'axis_%s' % first_axis)
        widget.focus()
        vars.ja_rbutton.set(first_axis)
    lastMotionMode = vars.motion_mode.get()


rC('grid','forget',fmanual + '.axis')
rC('grid','forget',fmanual + '.jogf')
rC('grid','forget',fmanual + '.space1')
#rC('grid','forget',fmanual + '.spindlel')
#rC('grid','forget',fmanual + '.spindlef')
rC('grid','forget',fmanual + '.space2')
rC('grid','forget',fmanual + '.coolant')
rC('grid','forget',fmanual + '.mist')
rC('grid','forget',fmanual + '.flood')

# destroy existing axes and joints
rC('destroy',faxes)
rC('destroy',fjoints)
# create widgets
rC('labelframe',faxes,'-text',_('Axis:'),'-relief','flat','-bd',0)
rC('labelframe',fjoints,'-text',_('Joint:'),'-relief','flat','-bd',0)
# make joints radiobuttons
for number in range(0,linuxcnc.MAX_JOINTS):
    ja_button_setup(fjoints + '.joint' + str(number), number, number)
# populate joints frame
count = 0
for row in range(0,2):
    for column in range(0,5):
        if count == jointcount: break
        pad = (0,0) if column == 0 else (8,0)
        rC('grid',fjoints + '.joint' + str(count),'-row',row,'-column',column,'-padx',pad)
        count += 1
# make axis radiobuttons
for letter in 'xyzabcuvw':
    ja_button_setup(faxes + '.axis' + letter, letter, letter.upper())
# populate the axes frame
count = 0
letters = 'xyzabcuvw'
first_axis = ''
for row in range(0,2):
    for column in range(0,5):
        if letters[count] in trajcoordinates:
            if first_axis == '':
                first_axis = letters[count]
            pad = (0,0) if column == 0 else (8,0)
            rC('grid',faxes + '.axis' + letters[count],'-row',row,'-column',column,'-padx',pad)
        count += 1
        if count == 9: break

# rework the jogf frame
rC('destroy',fjogf)
# create the widgets
rC('frame',fjogf)
rC('labelframe',fjogf + '.jog','-text',_('Jog:'),'-relief','flat','-bd',0)
rC('button',fjogf + '.jog.jogminus','-command','if ![is_continuous] {jog_minus 1}','-height',1,'-width',1,'-text','-')
rC('button',fjogf + '.jog.jogplus','-command','if ![is_continuous] {jog_plus 1}','-height',1,'-width',1,'-text','+')
rC('combobox',fjogf + '.jog.jogincr','-editable',0,'-textvariable','jogincrement','-value',_('Continuous'),'-width',10)
rC(fjogf + '.jog.jogincr','list','insert','end',_('Continuous'))
if increments:
    rC(fjogf + '.jog.jogincr','list','insert','end',*increments)
rC('labelframe',fjogf + '.zerohome','-text',_('Zero:'),'-relief','flat','-bd',0)
rC('button',fjogf + '.zerohome.home','-command','home_joint','-height',1,'-width',8,'-padx',0)
rC('setup_widget_accel',fjogf + '.zerohome.home',_('Home Axis'))
rC('button',fjogf + '.zerohome.zero','-command','touch_off_system','-height',1,'-width',5,'-padx',0)
rC('setup_widget_accel',fjogf + '.zerohome.zero','X0')
rC('button',fjogf + '.zerohome.zeroxy','-height',1,'-width',5,'-text',_('X0Y0'),'-padx',0)
rC('button',fjogf + '.zerohome.laser','-height',1,'-width',5,'-textvariable','laserText','-padx',0)
rC('button',fjogf + '.zerohome.tooltouch','-height',1,'-text','Tool Touch','-command','touch_off_tool','-padx',0) # unused... kept for tk hierarchy
# widget bindings
rC('bind',fjogf + '.jog.jogminus','<Button-1>','if [is_continuous] {jog_minus}')
rC('bind',fjogf + '.jog.jogminus','<ButtonRelease-1>','if [is_continuous] {jog_stop}')
rC('bind',fjogf + '.jog.jogplus','<Button-1>','if [is_continuous] {jog_plus}')
rC('bind',fjogf + '.jog.jogplus','<ButtonRelease-1>','if [is_continuous] {jog_stop}')
rC('bind',fjogf + '.zerohome.zeroxy','<Button-1>','touch_off_xy 1 0 0')
rC('bind',fjogf + '.zerohome.zeroxy','<ButtonRelease-1>','touch_off_xy 0 0 0')
rC('bind',fjogf + '.zerohome.laser','<Button-1>','laser_button_toggled 1 1')
rC('bind',fjogf + '.zerohome.laser','<ButtonRelease-1>','laser_button_toggled 0 1')
# populate the frame
rC('grid',fjogf + '.jog.jogminus','-row',0,'-column',0,'-sticky','nsew')
rC('grid',fjogf + '.jog.jogincr','-row',0,'-column',1,'-sticky','nsew','-padx',8)
rC('grid',fjogf + '.jog.jogplus','-row',0,'-column',2,'-sticky','nsew')
rC('grid',fjogf + '.jog','-row',0,'-column',0,'-sticky','ew')
rC('grid',fjogf + '.zerohome.home','-row',0,'-column',0,'-padx',(0,4))
rC('grid',fjogf + '.zerohome.zero','-row',0,'-column',1,'-padx',(0,4))
rC('grid',fjogf + '.zerohome.zeroxy','-row',0,'-column',2,'-padx',(0,4))
rC('grid',fjogf + '.zerohome.tooltouch','-row',0,'-column',3,'-padx',(0,4))
rC('grid',fjogf + '.zerohome','-row',1,'-column',0,'-pady',(2,0),'-sticky','w')
rC('grid',fjogf,'-column',0,'-row',1,'-padx',2,'-pady',(0,0),'-sticky','w','-columnspan',2)


# make home button a home all button if required
if homing_order_defined:
    if ja_name.startswith('A'):
        hbName = 'axes'
    else:
        hbName ='joints'
    widgets.homebutton.configure(text = _('Home All'), command = 'home_all_joints')
else:
    widgets.homebutton.configure(text = _('Home') + ' X' )

rC('button',fjogf + '.override') # dummy button to placate original axis code



################    end manual tab




# keep tab label sizes the same
#rC(ftabs,'configure','-homogeneous',True)
#rC(fright,'configure','-homogeneous',True)

# reduce margins on manual/mdi tabs
rC(ftabs,'configure','-internalborderwidth',5)
#rC('grid','columnconfigure',fmanual,0,'-weight',1)
#rC('grid','columnconfigure',fmanual,99,'-weight',0)

# reduce margins on preview tabs
rC(fright,'configure','-internalborderwidth',0)

# new setup toolbar
# create widgets
rC('frame','.toolsetup','-borderwidth',1,'-relief','raised')
rC('Button','.toolsetup.save','-command','save_setup_clicked','-width',8,'-takefocus',0,'-text',_('Save All'),'-padx',0)
rC('Button','.toolsetup.reload','-command','load_setup_clicked','-width',8,'-takefocus',0,'-text',_('Reload'),'-padx',0)
rC('Button','.toolsetup.add','-command','user_button_add','-width',8,'-takefocus',0,'-text',_('Add'),'-padx',0)
rC('Button','.toolsetup.bkp','-command','backup_clicked','-width',8,'-takefocus',0,'-text',_('Backup'),'-padx',0)
rC('Button','.toolsetup.close','-command','setup_toggle 0','-width',8,'-takefocus',0,'-text',_('Close'),'-padx',0)
# populate the tool bar
rC('pack','.toolsetup.save','-side','left')
rC('pack','.toolsetup.reload','-side','left','-padx',(8,0))
rC('pack','.toolsetup.add','-side','left','-padx',(8,0))
rC('pack','.toolsetup.bkp','-side','left','-padx',(8,0))
rC('pack','.toolsetup.close','-side','left','-padx',(16,0))
rC('grid','forget','.toolsetup')

# new settings frame
rC('frame',fsetup)
#####################################################################################
####  SETUP TABS
###########################################################################################

rC('NoteBook',fsetup + '.tabs')
rC(fsetup + '.tabs','insert','end','gui','-text',' GUI ')
rC(fsetup + '.tabs','insert','end','butt','-text',' BUTT ')
rC('pack',fsetup + '.tabs','-fill','both','-expand',1)
rC(fsetup + '.tabs','configure','-arcradius','8','-homogeneous',True,'-tabpady','1''1')

rC(fsetup + '.tabs','raise','gui')


############################################################################################
# left panel
rC('frame',fsetup + '.tabs.fgui.l')
# gui frame
rC('labelframe',fsetup + '.tabs.fgui.l.gui','-text','GUI','-relief','groove')
rC('label',fsetup + '.tabs.fgui.l.gui.closedialogL','-text','Close Dialog','-anchor','e')
rC('checkbutton',fsetup + '.tabs.fgui.l.gui.closedialog','-variable','closeDialog','-width',2,'-anchor','w','-indicatoron',0)
rC('label',fsetup + '.tabs.fgui.l.gui.wsizeL','-text','Window Size','-width', 13,'-anchor','e')
rC('ComboBox',fsetup + '.tabs.fgui.l.gui.wsize','-modifycmd','set_window_size','-textvariable','winSize','-bd',1,'-width',10,'-justify','right','-editable',0)
#rC(fsetup + '.tabs.fgui.l.gui.wsize','configure','-values',['default','last','fullscreen','maximized','small','medium'])
rC(fsetup + '.tabs.fgui.l.gui.wsize','configure','-values',['default','small','medium'])

rC('label',fsetup + '.tabs.fgui.l.gui.wscreenL','-text','Window Screen','-width', 13,'-anchor','e')
rC('ComboBox',fsetup + '.tabs.fgui.l.gui.wscreen','-modifycmd','set_window_screen','-textvariable','winScreen','-bd',1,'-width',10,'-justify','right','-editable',0)
rC(fsetup + '.tabs.fgui.l.gui.wscreen','configure','-values',['default','last','fullscreen','maximized'])


rC('label',fsetup + '.tabs.fgui.l.gui.fsizeL','-text','Font Size','-anchor','e')
rC('ComboBox',fsetup + '.tabs.fgui.l.gui.fsize','-modifycmd','font_size_changed','-textvariable','fontSize','-bd',1,'-width',10,'-justify','right','-editable',0)
rC(fsetup + '.tabs.fgui.l.gui.fsize','configure','-values',[9,10,11,12,13,14,15,16])
#rC('label',fsetup + '.tabs.fgui.l.gui.coneL','-text','Cone Size','-anchor','e')
#rC('ComboBox',fsetup + '.tabs.fgui.l.gui.cone','-modifycmd','cone_size_changed','-textvariable','coneSize','-bd',1,'-width',10,'-justify','right','-editable',0)
#rC(fsetup + '.tabs.fgui.l.gui.cone','configure','-values',[0.1,0.2,0.3,0.4,0.5,0.6,0.7,0.8,0.9,1.0])
#rC('label',fsetup + '.tabs.fgui.l.gui.zoomL','-text','Table Zoom','-anchor','e')
#rC('spinbox',fsetup + '.tabs.fgui.l.gui.zoom','-width', 10,'-justify','right','-wrap','true','-from',0.1,'-to',10.0,'-increment',0.1,'-format','%0.1f')
#rC(fsetup + '.tabs.fgui.l.gui.zoom','configure','-validate','key','-vcmd','{} %W {} {} %P %s'.format(valspin,'flt',1))
#spinBoxes.append(fsetup + '.tabs.fgui.l.gui.zoom')
#rC('label',fsetup + '.tabs.fgui.l.gui.kbShortcutsL','-text','Use KB Shortcuts','-anchor','e')
#rC('checkbutton',fsetup + '.tabs.fgui.l.gui.kbShortcuts','-variable','kbShortcuts','-command','kb_shortcuts_changed','-width',2,'-anchor','w','-indicatoron',0)
rC('label',fsetup + '.tabs.fgui.l.gui.loadlastL','-text','Load Last File','-anchor','e')
rC('checkbutton',fsetup + '.tabs.fgui.l.gui.loadlast','-variable','load_last','-width',2,'-anchor','w','-indicatoron',0)
rC('label',fsetup + '.tabs.fgui.l.gui.screensmallL','-text','Show Something','-anchor','e')
rC('checkbutton',fsetup + '.tabs.fgui.l.gui.screensmall','-variable','set_screensmall','-width',2,'-anchor','w','-indicatoron',0)
rC('label',fsetup + '.tabs.fgui.l.gui.hidejogL','-text','Hide Jog','-anchor','e')
rC('checkbutton',fsetup + '.tabs.fgui.l.gui.hidejog','-variable','hidejog','-command','hide_jog','-width',2,'-anchor','w','-indicatoron',0)
rC('label',fsetup + '.tabs.fgui.l.gui.hiderapidL','-text','Hide Rapid','-anchor','e')
rC('checkbutton',fsetup + '.tabs.fgui.l.gui.hiderapid','-variable','hiderapid','-command','hide_rapid','-width',2,'-anchor','w','-indicatoron',0)
rC('label',fsetup + '.tabs.fgui.l.gui.hidevelL','-text','Hide Max Vel.','-anchor','e')
rC('checkbutton',fsetup + '.tabs.fgui.l.gui.hidevel','-variable','hidevel','-command','hide_velocity','-width',2,'-anchor','w','-indicatoron',0)
rC('label',fsetup + '.tabs.fgui.l.gui.buttlabL','-text','Show Button Label','-anchor','e')
rC('checkbutton',fsetup + '.tabs.fgui.l.gui.buttlab','-variable','buttlab','-command','toolbar_config','-width',2,'-anchor','w','-indicatoron',0)
rC('label',fsetup + '.tabs.fgui.l.gui.buttflabL','-text','Hide Button Frame','-anchor','e')
rC('checkbutton',fsetup + '.tabs.fgui.l.gui.buttflab','-variable','hidebuttf','-width',2,'-anchor','w','-indicatoron',0)
#rC('checkbutton',fsetup + '.tabs.fgui.l.gui.buttflab','-variable','hidebuttf','-command','hide_buttonframe','-width',2,'-anchor','w','-indicatoron',0)


# populate gui frame
rC('grid',fsetup + '.tabs.fgui.l.gui.closedialogL','-column',0,'-row',0,'-sticky','e','-padx',(4,0),'-pady',(4,0))
rC('grid',fsetup + '.tabs.fgui.l.gui.closedialog','-column',1,'-row',0,'-sticky','e','-padx',(0,4),'-pady',(4,0))
rC('grid',fsetup + '.tabs.fgui.l.gui.wsizeL','-column',0,'-row',1,'-sticky','e','-padx',(4,0),'-pady',(4,0))
rC('grid',fsetup + '.tabs.fgui.l.gui.wsize','-column',1,'-row',1,'-sticky','e','-padx',(0,4),'-pady',(4,0))
rC('grid',fsetup + '.tabs.fgui.l.gui.wscreenL','-column',0,'-row',2,'-sticky','e','-padx',(4,0),'-pady',(4,0))
rC('grid',fsetup + '.tabs.fgui.l.gui.wscreen','-column',1,'-row',2,'-sticky','e','-padx',(0,4),'-pady',(4,0))
rC('grid',fsetup + '.tabs.fgui.l.gui.fsizeL','-column',0,'-row',3,'-sticky','e','-padx',(4,0),'-pady',(4,4))
rC('grid',fsetup + '.tabs.fgui.l.gui.fsize','-column',1,'-row',3,'-sticky','e','-padx',(0,4),'-pady',(4,4))
# ~ #rC('grid',fsetup + '.tabs.fgui.l.gui.coneL','-column',0,'-row',3,'-sticky','e','-padx',(4,0),'-pady',(4,4))
# ~ #rC('grid',fsetup + '.tabs.fgui.l.gui.cone','-column',1,'-row',3,'-sticky','e','-padx',(0,4),'-pady',(4,4))
# ~ #rC('grid',fsetup + '.tabs.fgui.l.gui.zoomL','-column',0,'-row',4,'-sticky','e','-padx',(4,0),'-pady',(4,4))
# ~ #rC('grid',fsetup + '.tabs.fgui.l.gui.zoom','-column',1,'-row',4,'-sticky','e','-padx',(0,4),'-pady',(4,4))
# ~ rC('grid',fsetup + '.tabs.fgui.l.gui.loadlastL','-column',0,'-row',5,'-sticky','e','-padx',(4,0),'-pady',(4,4))
# ~ rC('grid',fsetup + '.tabs.fgui.l.gui.loadlast','-column',1,'-row',5,'-sticky','e','-padx',(0,4),'-pady',(4,4))
# ~ #rC('grid',fsetup + '.tabs.fgui.l.gui.kbShortcutsL','-column',0,'-row',5,'-sticky','e','-padx',(4,0),'-pady',(4,4))
# ~ #rC('grid',fsetup + '.tabs.fgui.l.gui.kbShortcuts','-column',1,'-row',5,'-sticky','e','-padx',(0,4),'-pady',(4,4))
# ~ rC('grid',fsetup + '.tabs.fgui.l.gui.hidejogL','-column',0,'-row',7,'-sticky','e','-padx',(4,0),'-pady',(4,4))
# ~ rC('grid',fsetup + '.tabs.fgui.l.gui.hidejog','-column',1,'-row',7,'-sticky','e','-padx',(0,4),'-pady',(4,4))
# ~ rC('grid',fsetup + '.tabs.fgui.l.gui.hiderapidL','-column',0,'-row',8,'-sticky','e','-padx',(4,0),'-pady',(4,4))
# ~ rC('grid',fsetup + '.tabs.fgui.l.gui.hiderapid','-column',1,'-row',8,'-sticky','e','-padx',(0,4),'-pady',(4,4))
# ~ rC('grid',fsetup + '.tabs.fgui.l.gui.hidevelL','-column',0,'-row',9,'-sticky','e','-padx',(4,0),'-pady',(4,4))
# ~ rC('grid',fsetup + '.tabs.fgui.l.gui.hidevel','-column',1,'-row',9,'-sticky','e','-padx',(0,4),'-pady',(4,4))
rC('grid',fsetup + '.tabs.fgui.l.gui.buttlabL','-column',0,'-row',10,'-sticky','e','-padx',(4,0),'-pady',(4,4))
rC('grid',fsetup + '.tabs.fgui.l.gui.buttlab','-column',1,'-row',10,'-sticky','e','-padx',(0,4),'-pady',(4,4))
# ~ rC('grid',fsetup + '.tabs.fgui.l.gui.buttflabL','-column',0,'-row',11,'-sticky','e','-padx',(4,0),'-pady',(4,4))
# ~ rC('grid',fsetup + '.tabs.fgui.l.gui.buttflab','-column',1,'-row',11,'-sticky','e','-padx',(0,4),'-pady',(4,4))
rC('grid','columnconfigure',fsetup + '.tabs.fgui.l.gui',0,'-weight',1)
#populate left panel
rC('grid',fsetup + '.tabs.fgui.l.gui','-column',0,'-row',0,'-sticky','new')

#######################################################################################
#######################################################################################
# left2 panel
rC('frame',fsetup + '.tabs.fgui.l2')
# gui frame
rC('labelframe',fsetup + '.tabs.fgui.l2.gui','-text','CNC GUI','-relief','groove')
rC('label',fsetup + '.tabs.fgui.l2.gui.loadlastL','-text','Load Last File','-anchor','e')
rC('checkbutton',fsetup + '.tabs.fgui.l2.gui.loadlast','-variable','load_last','-width',2,'-anchor','w','-indicatoron',0)
rC('label',fsetup + '.tabs.fgui.l2.gui.hidejogL','-text','Hide Jog','-anchor','w')
rC('checkbutton',fsetup + '.tabs.fgui.l2.gui.hidejog','-variable','hidejog','-command','hide_jog','-width',2,'-anchor','w','-indicatoron',0)
rC('label',fsetup + '.tabs.fgui.l2.gui.hiderapidL','-text','Hide Rapid','-anchor','w')
rC('checkbutton',fsetup + '.tabs.fgui.l2.gui.hiderapid','-variable','hiderapid','-command','hide_rapid','-width',2,'-anchor','w','-indicatoron',0)
rC('label',fsetup + '.tabs.fgui.l2.gui.hidevelL','-text','Hide Max Vel.','-anchor','w')
rC('checkbutton',fsetup + '.tabs.fgui.l2.gui.hidevel','-variable','hidevel','-command','hide_velocity','-width',2,'-anchor','w','-indicatoron',0)
rC('label',fsetup + '.tabs.fgui.l2.gui.buttlabL','-text','Show Button Label','-anchor','w')
rC('checkbutton',fsetup + '.tabs.fgui.l2.gui.buttlab','-variable','buttlab','-command','toolbar_config','-width',2,'-anchor','w','-indicatoron',0)
rC('label',fsetup + '.tabs.fgui.l2.gui.buttflabL','-text','Hide Button Frame','-anchor','w')
rC('checkbutton',fsetup + '.tabs.fgui.l2.gui.buttflab','-variable','hidebuttf','-width',2,'-anchor','w','-indicatoron',0)
#rC('checkbutton',fsetup + '.tabs.fgui.l2.gui.buttflab','-variable','hidebuttf','-command','hide_buttonframe','-width',2,'-anchor','w','-indicatoron',0)


# populate cncgui frame
rC('grid',fsetup + '.tabs.fgui.l2.gui.loadlastL','-column',0,'-row',5,'-sticky','e','-padx',(4,0),'-pady',(4,4))
rC('grid',fsetup + '.tabs.fgui.l2.gui.loadlast','-column',1,'-row',5,'-sticky','e','-padx',(0,4),'-pady',(4,4))
rC('grid',fsetup + '.tabs.fgui.l2.gui.hidejogL','-column',0,'-row',7,'-sticky','e','-padx',(4,0),'-pady',(4,4))
rC('grid',fsetup + '.tabs.fgui.l2.gui.hidejog','-column',1,'-row',7,'-sticky','e','-padx',(0,4),'-pady',(4,4))
rC('grid',fsetup + '.tabs.fgui.l2.gui.hiderapidL','-column',0,'-row',8,'-sticky','e','-padx',(4,0),'-pady',(4,4))
rC('grid',fsetup + '.tabs.fgui.l2.gui.hiderapid','-column',1,'-row',8,'-sticky','e','-padx',(0,4),'-pady',(4,4))
rC('grid',fsetup + '.tabs.fgui.l2.gui.hidevelL','-column',0,'-row',9,'-sticky','e','-padx',(4,0),'-pady',(4,4))
rC('grid',fsetup + '.tabs.fgui.l2.gui.hidevel','-column',1,'-row',9,'-sticky','e','-padx',(0,4),'-pady',(4,4))
#rC('grid',fsetup + '.tabs.fgui.l2.gui.buttlabL','-column',0,'-row',10,'-sticky','e','-padx',(4,0),'-pady',(4,4))
#rC('grid',fsetup + '.tabs.fgui.l2.gui.buttlab','-column',1,'-row',10,'-sticky','e','-padx',(0,4),'-pady',(4,4))
rC('grid',fsetup + '.tabs.fgui.l2.gui.buttflabL','-column',0,'-row',11,'-sticky','e','-padx',(4,0),'-pady',(4,4))
rC('grid',fsetup + '.tabs.fgui.l2.gui.buttflab','-column',1,'-row',11,'-sticky','e','-padx',(0,4),'-pady',(4,4))
rC('grid','columnconfigure',fsetup + '.tabs.fgui.l2.gui',0,'-weight',1)
#populate left panel
rC('grid',fsetup + '.tabs.fgui.l2.gui','-column',0,'-row',0,'-sticky','new')


#########################################################################################
# middle panel for utilities
rC('frame',fsetup + '.tabs.fgui.m')
# utilities frame
#rC('labelframe',fsetup + '.tabs.fgui.m.utilities','-text','Utilities','-relief','groove')
#rC('button',fsetup + '.tabs.fgui.m.utilities.offsets','-command','set_peripheral_offsets','-text','Set Offsets')
# populate utilities frame
#rC('grid',fsetup + '.tabs.fgui.m.utilities.offsets','-column',0,'-row',0,'-sticky','new','-padx',4,'-pady',(0,4))
#rC('grid','columnconfigure',fsetup + '.tabs.fgui.m.utilities',0,'-weight',1)
# color frame
rC('labelframe',fsetup + '.tabs.fgui.m.colors','-text','Colors','-relief','groove')
rC('label',fsetup + '.tabs.fgui.m.colors.foreL','-width', 13,'-text','Foreground','-anchor','e')
rC('button',fsetup + '.tabs.fgui.m.colors.fore','-command','color_set fore')
rC('label',fsetup + '.tabs.fgui.m.colors.backL','-width', 13,'-text','Background','-anchor','e')
rC('button',fsetup + '.tabs.fgui.m.colors.back','-command','color_set back')
rC('label',fsetup + '.tabs.fgui.m.colors.disableL','-width', 13,'-text','Disabled','-anchor','e')
rC('button',fsetup + '.tabs.fgui.m.colors.disable','-command','color_set disable')
rC('label',fsetup + '.tabs.fgui.m.colors.activeL','-width', 13,'-text','Active','-anchor','e')
rC('button',fsetup + '.tabs.fgui.m.colors.active','-command','color_set active')
rC('label',fsetup + '.tabs.fgui.m.colors.warnL','-width', 13,'-text','Warning','-anchor','e')
rC('button',fsetup + '.tabs.fgui.m.colors.warn','-command','color_set warn')
rC('label',fsetup + '.tabs.fgui.m.colors.voltL','-width', 13,'-text','Label Value','-anchor','e')
rC('button',fsetup + '.tabs.fgui.m.colors.volt','-command','color_set volt')
# populate color frame
rC('grid',fsetup + '.tabs.fgui.m.colors.foreL','-column',0,'-row',0,'-sticky','e','-padx',4,'-pady',(4,4))
rC('grid',fsetup + '.tabs.fgui.m.colors.fore','-column',1,'-row',0,'-sticky','e','-padx',(0,4),'-pady',(4,4))
rC('grid',fsetup + '.tabs.fgui.m.colors.backL','-column',0,'-row',1,'-sticky','e','-padx',4,'-pady',(4,4))
rC('grid',fsetup + '.tabs.fgui.m.colors.back','-column',1,'-row',1,'-sticky','e','-padx',(0,4),'-pady',(4,4))
rC('grid',fsetup + '.tabs.fgui.m.colors.disableL','-column',0,'-row',2,'-sticky','e','-padx',4,'-pady',(4,4))
rC('grid',fsetup + '.tabs.fgui.m.colors.disable','-column',1,'-row',2,'-sticky','e','-padx',(0,4),'-pady',(4,4))
rC('grid',fsetup + '.tabs.fgui.m.colors.activeL','-column',0,'-row',3,'-sticky','e','-padx',4,'-pady',(4,4))
rC('grid',fsetup + '.tabs.fgui.m.colors.active','-column',1,'-row',3,'-sticky','e','-padx',(0,4),'-pady',(4,4))
rC('grid',fsetup + '.tabs.fgui.m.colors.warnL','-column',0,'-row',4,'-sticky','e','-padx',4,'-pady',(4,4))
rC('grid',fsetup + '.tabs.fgui.m.colors.warn','-column',1,'-row',4,'-sticky','e','-padx',(0,4),'-pady',(4,4))
rC('grid',fsetup + '.tabs.fgui.m.colors.voltL','-column',0,'-row',5,'-sticky','e','-padx',4,'-pady',(4,4))
rC('grid',fsetup + '.tabs.fgui.m.colors.volt','-column',1,'-row',5,'-sticky','e','-padx',(0,4),'-pady',(4,4))
rC('grid','columnconfigure',fsetup + '.tabs.fgui.m.colors',0,'-weight',1)
# populate middle panel
#rC('grid',fsetup + '.tabs.fgui.m.utilities','-column',0,'-row',0,'-sticky','new')
rC('grid',fsetup + '.tabs.fgui.m.colors','-column',0,'-row',1,'-sticky','new')

# right panel for text entries
rC('labelframe',fsetup + '.tabs.fbutt.r','-text','User Buttons','-relief','groove')
# frame for torch enable

# frame for user buttons
rC('ScrollableFrame',fsetup + '.tabs.fbutt.r.ubuttons','-bg',colorBack)
rC('scrollbar',fsetup + '.tabs.fbutt.r.yscroll','-orient','vertical','-command',fsetup + '.tabs.fbutt.r.ubuttons yview')
rC(fsetup + '.tabs.fbutt.r.ubuttons','configure','-yscrollcommand',fsetup + '.tabs.fbutt.r.yscroll set')
rC('pack',fsetup + '.tabs.fbutt.r.yscroll','-side','left','-fill','y')

# user button widgets
rC('label',fsetup + '.tabs.fbutt.r.ubuttons.frame.numL','-text',' #','-width',2,'-anchor','e')
rC('label',fsetup + '.tabs.fbutt.r.ubuttons.frame.nameL','-text','Name','-width',14,'-anchor','w')
rC('label',fsetup + '.tabs.fbutt.r.ubuttons.frame.codeL','-text','Code','-width',40,'-anchor','w')
for n in range(1, 20):
    rC('label',fsetup + '.tabs.fbutt.r.ubuttons.frame.num' + str(n),'-text',str(n),'-anchor','e')
    rC('entry',fsetup + '.tabs.fbutt.r.ubuttons.frame.name' + str(n),'-bd',1,'-width',14)
    rC('entry',fsetup + '.tabs.fbutt.r.ubuttons.frame.code' + str(n),'-bd',1,'-width',40)
rC('grid',fsetup + '.tabs.fbutt.r.ubuttons.frame.numL','-column',0,'-row',0,'-sticky','ne','-padx',(4,0))
rC('grid',fsetup + '.tabs.fbutt.r.ubuttons.frame.nameL','-column',1,'-row',0,'-sticky','nw','-padx',(4,0))
rC('grid',fsetup + '.tabs.fbutt.r.ubuttons.frame.codeL','-column',2,'-row',0,'-sticky','nw','-padx',(4,4))
# populate right panel
#rC('grid',fsetup + '.tabs.fbutt.r.torch','-column',0,'-row',0,'-sticky','new')
#rC('grid',fsetup + '.tabs.fbutt.r.ubuttons','-column',0,'-row',1,'-sticky','nsew')
#rC('grid',fsetup + '.tabs.fbutt.r.shutdown','-column',0,'-row',2,'-sticky','new')
#rC('grid','rowconfigure',fsetup + '.tabs.fbutt.r',1,'-weight',1)

# populate settings frame
rC('grid',fsetup + '.tabs.fgui.l','-column',0,'-row',0,'-sticky','nw','-padx',(4,0),'-pady',(4,4))
rC('grid',fsetup + '.tabs.fgui.l2','-column',1,'-row',0,'-sticky','nw','-padx',(4,0),'-pady',(4,4))
rC('grid',fsetup + '.tabs.fgui.m','-column',2,'-row',0,'-sticky','nw','-padx',(4,0),'-pady',(4,4))
#rC('grid',fsetup + '.tabs.fbutt.r','-column',0,'-row',0,'-sticky','ne','-padx',(0,4),'-pady',(4,4))
rC('pack',fsetup + '.tabs.fbutt.r.ubuttons','-fill','both','-side','left','-expand',1)
rC('pack',fsetup + '.tabs.fbutt.r','-fill','both','-side','left','-expand',1)
rC('grid','columnconfigure',fsetup,0,'-weight',0)
rC('grid','columnconfigure',fsetup,1,'-weight',0)
rC('grid','columnconfigure',fsetup,2,'-weight',0)
#rC('grid','columnconfigure',fsetup,3,'-weight',1)
#rC('grid','columnconfigure',fsetup,4,'-weight',0)


# new button panel
# create widgets
rC('frame','.fbuttons','-relief','flat')

# populate frame
rC('grid','.fbuttons','-column',0,'-row',3,'-rowspan',2,'-sticky','nsew','-padx',0,'-pady',0)
rC('grid','configure','.fbuttons','-sticky','nesw')


############################################################
##############   setup   #########'##############################
################### end  #########'
############################################################

##############################################################################
# INITIALIZATION                                                             #
##############################################################################

# reinitialize notifications to keep them on top of new widgets
notifications.__init__(root_window)
#_prompt_areyousure.__init__(root_window)
#_prompt_touchoff.__init__(root_window)
#_prompt_float__init__(root_window)

value = getPrefs(PREF,'GUI_OPTIONS', 'Exit warning', True, bool)
pVars.closeDialog.set(value)
restoreSetup['closeDialog'] = value
root_window.protocol('WM_DELETE_WINDOW', close_window)

value = getPrefs(PREF,'GUI_OPTIONS', 'Window size', 'default', str).lower().replace(' ','')
putPrefs(PREF,'GUI_OPTIONS', 'Window size', value, str)
pVars.winSize.set(value)
restoreSetup['winSize'] = value

value = getPrefs(PREF,'GUI_OPTIONS', 'Window screen', 'default', str).lower().replace(' ','')
putPrefs(PREF,'GUI_OPTIONS', 'Window screen', value, str)
pVars.winScreen.set(value)
restoreSetup['winScreen'] = value

value = getPrefs(PREF,'GUI_OPTIONS', 'Hide Max Jog. Slider', False, bool)
pVars.hidejog.set(value)
restoreSetup['hidejog'] = value
hide_jog()

value = getPrefs(PREF,'GUI_OPTIONS', 'Hide Max Rapid. Slider', False, bool)
pVars.hiderapid.set(value)
restoreSetup['hiderapid'] = value
hide_rapid()

value = getPrefs(PREF,'GUI_OPTIONS', 'Hide Max Velocity Slider', False, bool)
pVars.hidevel.set(value)
restoreSetup['hidevel'] = value
hide_velocity()
value = getPrefs(PREF,'GUI_OPTIONS', 'Show Button Label', False, bool)
pVars.buttlab.set(value)
restoreSetup['buttlab'] = value
toolbar_config()
value = getPrefs(PREF,'GUI_OPTIONS', 'Load last file', False, bool)
pVars.load_last.set(value)
restoreSetup['load_last'] = value

pVars.runS.set(int(getPrefs(PREF,'STATISTICS', 'Program run time', 0, float)))
pVars.runJ.set('00:00:00')
pVars.runT.set(secs_to_hms(pVars.runS.get()))

value = getPrefs(PREF,'GUI_OPTIONS', 'Hide Button Frame', False, bool)
pVars.hidebuttf.set(value)
restoreSetup['hidebuttf'] = value
#hide_buttonframe()
#print(hal.get_info_pins())
get_coordinate_font = get_coordinate_font
halPinList = hal.get_info_pins()
user_button_load()
font_size_changed()
#color_change()
set_window_size()
hide_jog()
hide_rapid()
hide_velocity()
hide_buttonframe()



    
##########################################################
########       LOAS_LAST FILE   PY3  LCNC 2.9  #############
########   In ini file under [DISPLAY]  ##################
########     LOAD_LASTFILE = 	YES       ##################
##########################################################

#loadlast = inifile.find('DISPLAY', 'LOAD_LASTFILE')

if pVars.load_last.get() == True :
    load_lastfile = True
else:
    load_lastfile = False

lastfile = ""
recent = ap.getpref('recentfiles', [], repr)
if len(recent):
    lastfile = recent.pop(0)

code = []
addrecent = True
if args:
    initialfile = args[0]
elif "AXIS_OPEN_FILE" in os.environ:
    initialfile = os.environ["AXIS_OPEN_FILE"]
elif inifile.find("DISPLAY", "OPEN_FILE"):
    initialfile = inifile.find("DISPLAY", "OPEN_FILE")
elif os.path.exists(lastfile) and load_lastfile:
    initialfile = lastfile
    print ("Loading ") 
    print (initialfile)
elif lathe:
    initialfile = os.path.join(BASE, "share", "axis", "images","axis-lathe.ngc")
    addrecent = False
else:
    initialfile = os.path.join(BASE, "share", "axis", "images", "axis.ngc")
    addrecent = False

if os.path.exists(initialfile):
    open_file_guts(initialfile, False, addrecent)

###############################
######    end load last    ###############
#########################################

##############################################################################
# HAL SETUP - CALLED DIRECTLY FROM AXIS ONCE AT STARTUP                      #
##############################################################################
def user_hal_pins():
    # do user button setup after hal pin creation
    user_button_setup()
    color_change()
    
    

def get_button_num(name):
    num = None
    for num in buttonCodes:
        if buttonCodes[num]['code'] == name:
            break
    return num


#################################################
##### USER LIVE UPDATE ############################
#####  this is where pins get updated in ui  ###########
##################################################

rC('.pane.top.tabs','itemconfigure','edit','-state','disabled')

def user_live_update():
   
   ## loaded file
    rC('.pane.top.tabs.fedit.program.name','configure','-text', os.path.basename(loaded_file))
    rC('.pane.top.tabs.fauto.program.name','configure','-text', os.path.basename(loaded_file))
    ## for edit tab  ##
    s.poll()
    if s.paused == True:
        rC(".toolbar.program_pause","configure","-state",'active')
    if s.task_mode == 2:
      #rC('.pane.top.tabs','itemconfigure','manual','-state','disabled')
      #rC('.pane.top.tabs','itemconfigure','mdi','-state','disabled')
      #rC('.pane.top.tabs','itemconfigure','edit','-state','disabled')
      #rC('.pane.top.tabs','itemconfigure','edit')
      #rC('.menu','entryconfig','Setup','-state','disabled')
      rC('.pane.top.tabs','raise','auto')
      tab_auto()
    else:
      tab_auto() 
      #rC('.pane.top.tabs','itemconfigure','manual','-state','normal')
      #rC('.pane.top.tabs','itemconfigure','mdi','-state','normal')
      #rC('.pane.top.tabs','itemconfigure','edit','-state','normal')
      #rC('.menu','entryconfig','Setup','-state','normal')
    if pVars.editmode.get() ==True:
        if pVars.editfile.get() != s.file:
            load_editfile()
            
    for key in togglePins:
        if hal.get_value(togglePins[key]['pin']) != togglePins[key]['state']:
            set_toggle_pins(togglePins[key])
        
    ####   stats
    #sNow = int(hal.get_value('plasmac.run-time') + 0.5)
    #if statValues['run'] != sNow:
    #    if sNow:
    #        pVars.runJ.set(secs_to_hms(sNow))
    #    statValues['run'] = sNow
      

#################################################
##### USER LIVE UPDATE ############################
#####         end             ######################
##################################################

TclCommands.ja_button_setup = ja_button_setup
TclCommands.ja_button_activated = ja_button_activated
TclCommands.joint_mode_switch = joint_mode_switch

TclCommands.keybind_edit = keybind_edit
TclCommands.keybind_edit_restore = keybind_edit_restore
TclCommands.edit_full = edit_full
TclCommands.edit_lower = edit_lower
TclCommands.edit_size = edit_size
TclCommands.edit_tab_lower = edit_tab_lower
TclCommands.edit_tab_raise = edit_tab_raise
TclCommands.auto_tab_lower = auto_tab_lower
TclCommands.auto_tab_raise = auto_tab_raise
TclCommands.tab_auto = tab_auto

TclCommands.button_action = button_action
TclCommands.user_button_add = user_button_add
TclCommands.set_toggle_pins = set_toggle_pins

TclCommands.reset_all_stats = reset_all_stats
TclCommands.close_window = close_window
TclCommands.load_setup_clicked = load_setup_clicked
TclCommands.save_setup_clicked = save_setup_clicked
TclCommands.font_size_changed = font_size_changed
TclCommands.hide_buttonframe = hide_buttonframe
TclCommands.hide_jog = hide_jog
TclCommands.hide_rapid = hide_rapid
TclCommands.hide_velocity = hide_velocity
TclCommands.toolbar_config = toolbar_config
TclCommands.set_screensmall = set_screensmall
TclCommands.screen_medium = screen_medium
TclCommands.set_window_size = set_window_size
TclCommands.set_window_screen = set_window_screen
TclCommands.read_colors = read_colors
TclCommands.color_user_buttons = color_user_buttons
TclCommands.color_change = color_change
TclCommands.color_set = color_set
TclCommands.set_screensmall = set_screensmall
TclCommands.setup_toggle = setup_toggle
TclCommands.show_default = show_default
TclCommands.hide_default = hide_default
TclCommands.enable_menus = enable_menus
TclCommands.save_editfile = save_editfile
TclCommands.load_editfile = load_editfile
TclCommands.printfile = print_file
commands = TclCommands(root_window)
