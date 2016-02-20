import socket
import sys
import time
import Tkinter as tk
import tkMessageBox
import tkFileDialog
import tkFont
import matplotlib.pyplot as plt
import matplotlib, sys
import numbers
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg, NavigationToolbar2TkAgg
from matplotlib.figure import Figure
#import matplotlib.pyplot as plt
import matplotlib.mlab as mlab
import numpy as np
from tkintertable import TableCanvas, TableModel
from Tkconstants import ANCHOR
matplotlib.use('TkAgg')

'''class child1(tk.Toplevel):
        def __init__(self,parent,func):
                self.func = func
                self.ok_button = tk.Button(parent, command=self.on_ok)
        def on_ok(self):
                self.func("button pressed in child1")
        def show(self):
'''
class mainApp(tk.Tk):
    def __init__(self):
        tk.Tk.__init__(self)
        self.geometry("600x300")
        self.configure(bg = '#888888')
        self.resizable(width='false', height='false')
        self.helv15 = tkFont.Font(family='Helvetica', size=15, weight='bold')
        self.helv10 = tkFont.Font(family='Helvetica', size=10, weight='bold')
        self.headerFont = tkFont.Font(family='Helvetica', size=20, weight='bold')
        #Create Menubar & Main Layout
        self.createMenuBar()
        self.setupLayout()
        #self.setupGUI()
        self.title("Finance App")

        self.connected = False
        self.setupSocket()
        self.recMSG = ""
        self.daily_change_clean = False
        self.risk_by_maturity_clean = False
        self.histogram_clean = False
        self.hist_data = []
    def setupSocket(self):
        self.HOST = socket.gethostname()
        print "hostname: ", self.HOST
        self.PORT = 1234
        self.MSGSIZE = 10000#81920
        self.s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    def doNothing(self):
        self.throw_warning("warning", "doing nothing")
        return
    def connectToServer(self):
        if self.connected == True:
            self.throw_warning("warning", "already connected")
            return
        self.s.connect((self.HOST, self.PORT))
        self.connected = True
    def disconnectFromServer(self):
        if self.connected == False:
            self.throw_warning("warning", "already disconnected")
            return
        self.s.close()
        self.connected = False
    def throw_warning(self, st1, st2):
        tkMessageBox.showwarning(st1, st2)
    def createMenuBar(self):
        # CREATE MENU
        self.mainMenu = tk.Menu(self)
        self.config(menu=self.mainMenu)

        # FILE SUBMENU
        self.subMenu_file = tk.Menu(self.mainMenu)
        self.mainMenu.add_cascade(label="File", menu=self.subMenu_file)
        self.subMenu_file.add_command(label="Option 1", command=self.doNothing)
        self.subMenu_file.add_command(label="Option 2", command=self.doNothing)
        self.subMenu_file.add_command(label="Option 3", command=self.doNothing)
        self.subMenu_file.add_command(label="Option 4", command=self.doNothing)
        self.subMenu_file.add_separator()
        self.subMenu_file.add_command(label="Exit", command=exit)

        # Settings
        self.subMenu_settings = tk.Menu(self.mainMenu)
        self.mainMenu.add_cascade(label="Settings", menu=self.subMenu_settings)
        self.subMenu_settings.add_command(label="Connect", command=self.connectToServer)
        self.subMenu_settings.add_command(label="Disconnect", command=self.disconnectFromServer)

        # Help
        self.subMenu_help = tk.Menu(self.mainMenu)
        self.mainMenu.add_cascade(label="Help", menu=self.subMenu_help)
        self.subMenu_help.add_command(label="Help", command=self.doNothing)
        self.subMenu_help.add_command(label="About", command=self.doNothing)
    def tbSummary(self):
        if self.connected == False:
            self.throw_warning("warning", "connect to server first: Settings -> Connect")
            return
        self.s.send('RUN')
        self.recMSG = self.s.recv(self.MSGSIZE)

        #Just spill received message on Server Response textbox
        self.text2_serverOut.insert('end',self.recMSG)
        #DEBUG
        for line in self.recMSG:
            print line
    def run2(self):
        if self.connected == False:
            self.throw_warning("warning", "connect to server first")
            return
        self.s.send('RUN2')
        self.recMSG = self.s.recv(self.MSGSIZE)
        #Just spill received message on Server Response textbox
        print self.recMSG
        self.text2_serverOut.insert('end',str(self.recMSG+'\n'))
    def run3(self):
        if self.connected == False:
            self.throw_warning("warning", "connect to server first")
            return
        self.s.send('RUN3')
        self.recMSG = self.s.recv(self.MSGSIZE)
        #Just spill received message on Server Response textbox
        self.text2_serverOut.insert('end',str(self.recMSG+'\n'))
    def processLines(self):
        all_lines=[]
        #from the stream of bytes, form a list(all_lines) of lines ending with marker "EOL" ex: [ [i am good EOL], [so are you EOL]]
        chstream=""
        for ch in self.recMSG:
            if ch != '\n':
                chstream += ch
            else:
                chstream+=" EOL"
                all_lines.append(chstream)
                chstream=""
        line_idx=0
        all_rows=[]#each row should be a list, so all_row is a list of lists
        for line in all_lines:
            row=[]
            print line
            words = line.split()
            for word in words:
                if word!="EOL":
                    row.append(word)
            all_rows.append(row)
        return all_rows
    def grab_hist_data(self):
        numbers=[]
        word = ""
        numbers = self.recMSG.split()
        numbers = numbers[1:]
        num = []
        for n in numbers:
            num.append(int(n))
        print "hist data: ", num
        return num

    def tbSummary2(self):
        if self.connected == False:
            self.throw_warning("warning", "connect to server first")
            return
        self.s.send('RUN')
        self.recMSG = self.s.recv(self.MSGSIZE)

        #Just spill received message on Server Response textbox
        self.text2_serverOut.insert('end',self.recMSG)

        self.all_lines=self.processLines()#[ ['a','b','c'], ['d','e','f'] ]
        self.grab_hist_data()
        self.text2_serverOut.insert('end',self.all_lines)
        print "Structured Data:"
        print self.all_lines
    def cleanUpBeforeExit(self,op):
        print "cleanup ",op
        if op==1 and self.daily_change_clean == True:
            self.daily_change_clean = False
            return
        if op==2 and self.risk_by_maturity_clean == True:
            self.risk_by_maturity_clean = False
            return
        if op==3 and self.histogram_clean == True:
            print "closed hist window"
            self.histogram_clean = False
            return
    def grab_daily_change(self):
        lines = []
        line = ""
        for ch in self.recMSG:
            if ch!='\n':
                line += ch
            else:
                lines.append(line)
                line = ""
        #print lines,"\n\n\n"
        opening_start_ticker_idx = lines.index('OpeningTicker')
        opening_end_ticker_idx = lines.index('OpeningQuality')
        opening_list_ticker = lines[opening_start_ticker_idx+1:opening_end_ticker_idx]
        #print opening_list_ticker,'\n\n\n'

        opening_start_quality_idx = lines.index('OpeningQuality')
        opening_end_quality_idx = lines.index('ClosingTicker')
        opening_list_quality = lines[opening_start_quality_idx+1:opening_end_quality_idx]
        #print opening_list_quality,'\n\n\n'

        closing_start_ticker_idx = lines.index('ClosingTicker')
        closing_end_ticker_idx = lines.index('ClosingQuality')
        closing_list_ticker = lines[closing_start_ticker_idx+1:closing_end_ticker_idx]
        #print closing_list_ticker,'\n\n\n'

        closing_start_quality_idx = lines.index('ClosingQuality')
        closing_end_quality_idx = lines.index('OpeningVaR')
        closing_list_quality = lines[closing_start_quality_idx+1:closing_end_quality_idx]
        #print closing_list_quality,'\n\n\n'

        opening_var_start_idx = lines.index('OpeningVaR')
        opening_var_end_index = lines.index('ClosingVaR')
        opening_list_var = lines[opening_var_start_idx+1:opening_var_end_index]
        print opening_list_var,'\n\n\n'

        closing_list_var = lines[lines.index('ClosingVaR')+1:-1]
        print closing_list_var,'\n\n\n'

        return (opening_list_ticker,opening_list_quality,closing_list_ticker,closing_list_quality,opening_list_var,closing_list_var)
    def spawn_daily_change(self):
        '''self.cd1 = child1(self,self.func)
        self.cd1.show()'''
        if self.connected == False:
            self.throw_warning("warning", "connect to server first")
            return
        '''if self.daily_change_clean == True:
                self.throw_warning("warning", "Daily change window is already open")
                return
        self.daily_change_clean = True'''
        self.s.send('RUN1')
        self.recMSG = self.s.recv(self.MSGSIZE)
        #print self.recMSG
        self.c2 = tk.Toplevel()
        self.c2.geometry("1080x600")
        data = self.grab_daily_change()
        #print "Toplevel"
        #self.c2.protocol('WM_DELETE_WINDOW',self.cleanUpBeforeExit(1))

        '''dic = {'k1':{'c1':'v1-1','c2':'v1-2','label':'k1'},'k2':{'c1':'v2-1','c2':'v2-2','label':'k2'}}

        tframe = tk.Frame(self.c2)
        tframe.pack()
        model = TableModel()
        model.importDict(dic)
        table = TableCanvas(tframe,model=model,reverseorder=1)
        table.createTableFrame()
        table.redrawTable()
        return'''
        self.c2.c2frame0 = tk.Frame(self.c2, bg='#EFEFEF', bd=4, width='1060', height='100', relief='sunken')
        self.c2.c2frame0.pack_propagate(False)
        self.c2.c2frame0.pack(side='top',padx=5,pady=5)
        self.c2.label01 = tk.Label(self.c2.c2frame0, fg = 'black', text="Daily Change by Issuer", font=self.headerFont)
        self.c2.label01.pack()


        self.c2.c2frame1 = tk.Frame(self.c2, bg='#DDDDDD', bd=2, width='350', height='400')

        self.c2.c2text1 = tk.Text(self.c2.c2frame1, bg='black', fg='#cccc00', bd=1, width='350', height='400', font=self.helv10)#, state='disabled')


        self.c2.c2frame1.pack_propagate(False)
        self.c2.c2text1.pack(expand='yes',fill='both')
        self.c2.c2frame1.pack(side='left',padx=5,pady=5)#,fill='none',expand='false')

        (op_t,op_q,cl_t,cl_q,op_v,cl_v) = self.grab_daily_change()
        #print op_v," ",len(op_v)
        #print cl_v," ",len(cl_v)
        tickers_ob = []
        qualities_ob = []
        ########### FILL UP OPEN BOOK TICKER SUMMARY #############
        self.c2.c2text1.insert('end',"\tOpening Book Summary - By Ticker\n")
        self.c2.c2text1.insert('end',"|Ticker\t|Amount\t\t|Risk\t\t|LGD\t|\n") #Opening Book by Ticker
        triplet_set_obt = []
        for line in op_t:
            #for each line, there will be 4 words: one id and three numbers; store the 3 numbers as a triplet
            triplet = []
            words = line.split()
            #print words
            #self.c2.c2text1.insert('end',line+'\n')
            cnt = 0
            for w in words:
                #print "word",w
                if cnt == 0:
                    tickers_ob.append(w)
                if cnt != 0:
                    triplet.append(w)
                cnt += 1
                if cnt > 2:
                    self.c2.c2text1.insert('end',"\t|"+w+"\t")
                else:
                    self.c2.c2text1.insert('end',"|"+w+"\t")
            triplet_set_obt.append(triplet)
            self.c2.c2text1.insert('end','|\n')
        #print triplet_set_obt #got correct data
        ########### FILL UP OPEN BOOK QUALITY SUMMARY #############
        self.c2.c2text1.insert('end',"\n\tOpening Book Summary - By Quality\n")
        self.c2.c2text1.insert('end',"|Quality\t|Amount\t\t|Risk\t\t|LGD\t|\n") #Opening Book by Quality
        triplet_set_obq = []
        for line in op_q:
            triplet = []
            words = line.split()
            #print words
            cnt = 0
            for w in words:
                #print "word",w
                if cnt == 0:
                    qualities_ob.append(w)
                if cnt != 0:
                    triplet.append(w)
                cnt += 1
                if cnt > 2:
                    self.c2.c2text1.insert('end',"\t|"+w+"\t")
                else:
                    self.c2.c2text1.insert('end',"|"+w+"\t")
            triplet_set_obq.append(triplet)
            self.c2.c2text1.insert('end','|\n')
        #print "triplet_set_obq\n",triplet_set_obq
        ########### FILL UP CLOSED BOOK TICKER SUMMARY #############
        self.c2.c2frame2 = tk.Frame(self.c2, bg='#DDDDDD', bd=2, width='350', height='400')
        self.c2.c2text2 = tk.Text(self.c2.c2frame2, bg='black', fg='#00cc00', bd=1, width='350', height='400', font=self.helv10)#, state='disabled')
        self.c2.c2frame2.pack_propagate(False)
        self.c2.c2text2.pack(expand='yes',fill='both')
        self.c2.c2frame2.pack(side='left',padx=5,pady=5)#,fill='none',expand='false')


        self.c2.c2text2.insert('end',"\tClosing Book Summary - By Ticker\n")
        self.c2.c2text2.insert('end',"|Ticker\t|Amount\t\t|Risk\t\t|LGD\t|\n") #Closing Book by Ticker

        tickers_cb = []
        qualities_cb = []
        triplet_set_cbt = []
        for line in cl_t:
            triplet = []
            words = line.split()
            #print words
            #self.c2.c2text1.insert('end',line+'\n')
            cnt = 0
            for w in words:
                #print "word",w
                if cnt == 0:
                    tickers_cb.append(w)
                if cnt != 0:
                    triplet.append(w)
                cnt += 1
                if cnt > 2:
                    self.c2.c2text2.insert('end',"\t|"+w+"\t")
                else:
                    self.c2.c2text2.insert('end',"|"+w+"\t")
            triplet_set_cbt.append(triplet)
            self.c2.c2text2.insert('end','|\n')
        #print "triplet set cbt\n",triplet_set_cbt
        ########### FILL UP CLOSED BOOK QUALITY SUMMARY #############
        self.c2.c2text2.insert('end',"\n\tClosing Book Summary - By Quality\n")
        self.c2.c2text2.insert('end',"|Quality\t|Amount\t\t|Risk\t\t|LGD\t|\n") #Opening Book by Quality
        triplet_set_cbq = []
        for line in cl_q:
            triplet = []
            words = line.split()
            #print words
            cnt = 0
            for w in words:
                #print "word",w
                if cnt == 0:
                    qualities_cb.append(w)
                if cnt != 0:
                    triplet.append(w)
                cnt += 1
                if cnt > 2:
                    self.c2.c2text2.insert('end',"\t|"+w+"\t")
                else:
                    self.c2.c2text2.insert('end',"|"+w+"\t")
            triplet_set_cbq.append(triplet)
            self.c2.c2text2.insert('end','|\n')
        #print "triplet set cbq\n",triplet_set_cbq
        ########### FILL UP CHANGE TICKER SUMMARY #############
        self.c2.c2frame3 = tk.Frame(self.c2, bg='#DDDDDD', bd=2, width='350', height='400')
        self.c2.c2text3 = tk.Text(self.c2.c2frame3, bg='black', fg='white', bd=1, width='350', height='400', font=self.helv10)#, state='disabled')
        self.c2.c2frame3.pack_propagate(False)
        self.c2.c2text3.pack(expand='yes',fill='both')
        self.c2.c2frame3.pack(side='left',padx=5,pady=5)#,fill='none',expand='false')
        self.c2.c2text3.insert('end',"\tChange Book Summary - By Ticker\n")
        self.c2.c2text3.insert('end',"|Ticker\t|Amount\t\t|Risk\t\t|LGD\t|\n") #Change Book by Ticker
        ## USE triplet_set_obt, triplet_set_cbt, tickers_ob, tickers_cb
        tickers_not_in_cb = []
        tickers_not_in_ob = []
        for x in tickers_ob:
            if x not in tickers_cb:
                tickers_not_in_cb.append(x)
        for x in tickers_cb:
            if x not in tickers_ob:
                tickers_not_in_ob.append(x)

        #print "tickers not in ob:",tickers_not_in_ob
        #print "tickers not in cb",tickers_not_in_cb
        #print "tickers in ob",tickers_ob
        #print "tickers in cb",tickers_cb

        for i in tickers_cb:
            if i in tickers_ob:
                idx_cb = tickers_cb.index(i)
                idx_ob = tickers_ob.index(i)
                triplet_cbt = triplet_set_cbt[idx_cb]
                triplet_obt = triplet_set_obt[idx_ob]
                #print "triplet cbt:",triplet_cbt
                #print "triplet obt:",triplet_obt
                #print "types:",type(triplet_obt[0])," ",type(triplet_obt[1])," ",type(triplet_obt[2])
                line = "|"+i+"\t|"+str(int(triplet_cbt[0])-int(triplet_obt[0])) +"\t\t|"+ str(float(triplet_cbt[1])-float(triplet_obt[1])) + "\t\t|" + str(float(triplet_cbt[2])-float(triplet_obt[2])) + "\t|\n"
                self.c2.c2text3.insert('end',line)
        #EDGE CASE 1
        for i in tickers_cb:
            if i not in tickers_ob:
                #print "ticker ",i," in closing but not in opening"
                idx_cb = tickers_cb.index(i)
                triplet_cbt = triplet_set_cbt[idx_cb]
                triplet_obt = ['0','0','0']
                #print "triplet cbt:",triplet_cbt
                #print "triplet obt:",triplet_obt
                line = "|"+i+"\t"+str(int(triplet_cbt[0])-int(triplet_obt[0])) +"\t\t|"+ str(float(triplet_cbt[1])-float(triplet_obt[1])) + "\t\t|" + str(float(triplet_cbt[2])-float(triplet_obt[2])) + "\t|\n"
                self.c2.c2text3.insert('end',line)
        #EDGE CASE 2
        for i in tickers_ob:
            if i not in tickers_cb:
                #print "ticker ",i," in opening but not in closing"
                idx_ob = tickers_ob.index(i)
                triplet_obt = triplet_set_obt[idx_ob]
                triplet_cbt = ['0','0','0']
                #print "triplet cbt:",triplet_cbt
                #print "triplet obt:",triplet_obt
                line = "|"+i+"\t"+str(int(triplet_cbt[0])-int(triplet_obt[0])) +"\t\t|"+ str(float(triplet_cbt[1])-float(triplet_obt[1])) + "\t\t|" + str(float(triplet_cbt[2])-float(triplet_obt[2])) + "\t|\n"
                self.c2.c2text3.insert('end',line)

        ########### FILL UP CHANGE QUALITY SUMMARY #############
        self.c2.c2text3.insert('end',"\n\tChange Book Summary - By Quality\n")
        self.c2.c2text3.insert('end',"|Quality\t|Amount\t\t|Risk\t\t|LGD\t|\n") #Change Book by Ticker
        ## USE triplet_set_obq, triplet_set_cbq, qualities_ob, qualities_cb
        qualities_not_in_cb = []
        qualities_not_in_ob = []
        for x in qualities_ob:
            if x not in qualities_cb:
                qualities_not_in_cb.append(x)
        for x in qualities_cb:
            if x not in qualities_ob:
                qualities_not_in_ob.append(x)

        #print "qualities not in ob:",qualities_not_in_ob
        #print "qualities not in cb",qualities_not_in_cb
        #print "qualities in ob",qualities_ob
        #print "qualities in cb",qualities_cb

        for i in qualities_cb:
            if i in qualities_ob:
                idx_cb = qualities_cb.index(i)
                idx_ob = qualities_ob.index(i)
                triplet_cbq = triplet_set_cbq[idx_cb]
                triplet_obq = triplet_set_obq[idx_ob]
                #print "triplet cbq:",triplet_cbq
                #print "triplet obq:",triplet_obq
                #print "types:",type(triplet_obq[0])," ",type(triplet_obq[1])," ",type(triplet_obq[2])
                line = "|"+i+"\t|"+str(int(triplet_cbq[0])-int(triplet_obq[0])) +"\t\t|"+ str(float(triplet_cbq[1])-float(triplet_obq[1])) + "\t\t|" + str(float(triplet_cbq[2])-float(triplet_obq[2])) + "\t|\n"
                self.c2.c2text3.insert('end',line)
        #EDGE CASE 1
        for i in qualities_cb:
            if i not in qualities_ob:
                #print "ticker ",i," in closing but not in opening"
                idx_cb = qualities_cb.index(i)
                triplet_cbq = triplet_set_cbq[idx_cb]
                triplet_obq = ['0','0','0']
                #print "triplet cbq:",triplet_cbq
                #print "triplet obq:",triplet_obq
                line = "|"+i+"\t|"+str(int(triplet_cbq[0])-int(triplet_obq[0])) +"\t\t|"+ str(float(triplet_cbq[1])-float(triplet_obq[1])) + "\t\t|" + str(float(triplet_cbq[2])-float(triplet_obq[2])) + "\t|\n"
                self.c2.c2text3.insert('end',line)
        #EDGE CASE 2
        for i in qualities_ob:
            if i not in qualities_cb:
                #print "ticker ",i," in opening but not in closing"
                idx_ob = qualities_ob.index(i)
                triplet_obq = triplet_set_obq[idx_ob]
                triplet_cbq = ['0','0','0']
                #print "triplet cbt:",triplet_cbq
                #print "triplet obt:",triplet_obq
                line = "|"+i+"\t|"+str(int(triplet_cbq[0])-int(triplet_obq[0])) +"\t\t|"+ str(float(triplet_cbq[1])-float(triplet_obq[1])) + "\t\t|" + str(float(triplet_cbq[2])-float(triplet_obq[2])) + "\t|\n"
                self.c2.c2text3.insert('end',line)


        ##############BOTTOM DATA VAR ###############
        #print type(op_v[0]),type(cl_v[0]) #total,yield,spread
        open_var = op_v[0].split()
        close_var = cl_v[0].split()
        diff_var = []
        i = 0
        while i<len(close_var):
            diff_var.append(str(float(close_var[i])-float(open_var[i])))
            i += 1
        #print close_var
        #print open_var
        #print diff_var
        self.c2.c2text1.insert('end',"\n\nVaR Credit(Spread):\t"+str(open_var[2])+'\n')
        self.c2.c2text1.insert('end',"VaR Rate(Yield):\t"+str(open_var[1])+'\n')
        self.c2.c2text1.insert('end',"VaR Total:\t"+str(open_var[0])+'\n')

        self.c2.c2text2.insert('end',"\n\nVaR Credit(Spread):\t"+str(close_var[2])+'\n')
        self.c2.c2text2.insert('end',"VaR Rate(Yield):\t"+str(close_var[1])+'\n')
        self.c2.c2text2.insert('end',"VaR Total:\t"+str(close_var[0])+'\n')

        self.c2.c2text3.insert('end',"\n\nVaR Credit(Spread):\t"+str(diff_var[2])+'\n')
        self.c2.c2text3.insert('end',"VaR Rate(Yield):\t"+str(diff_var[1])+'\n')
        self.c2.c2text3.insert('end',"VaR Total:\t"+str(diff_var[0])+'\n')

        return

        '''dic_op_t = {}
        for line in op_t:
                words = line.split()
                print words
                dic_op_t[words[0]]=words[1:]
                ###self.c2.c2text1.insert('end',line+'\n')
        print dic_op_t
        tframe = tk.Frame(self.c2)
        tframe.pack()
        model = TableModel()
        model.importDict(dic_op_t)
        table = TableCanvas(tframe,model=model,reverseorder=1)
        table.createTableFrame()
        table.redrawTable()
        return'''

        self.c2.c2text1.insert('end',"Opening Book\nQuality\tAmount\tRisk\tLGD\n")
        for line in op_q:
            self.c2.c2text1.insert('end',line+'\n')

        self.c2.c2text1.insert('end',"Closing Book\nTicker\tAmount\tRisk\tLGD\n")
        for line in cl_t:
            self.c2.c2text1.insert('end',line+'\n')

        self.c2.c2text1.insert('end',"Closing Book\nQuality\tAmount\tRisk\tLGD\n")
        for line in cl_q:
            self.c2.c2text1.insert('end',line+'\n')


        self.c2.c2text1.insert('end',"Opening Book\nPnL-VAR\tYield-VAR\tSpread-VAR\n")
        for line in op_v:
            self.c2.c2text1.insert('end',line+'\n')
        self.c2.c2text1.insert('end',"Closing Book\nPnL-VAR\tYield-VAR\tSpread-VAR\n")
        for line in cl_v:
            self.c2.c2text1.insert('end',line+'\n')
    def handle_rm(self):
        lines = []
        line = ""
        for ch in self.recMSG:
            if ch!='\n':
                line += ch;
            else:
                lines.append(line)
                line = ""
        #print lines
        return lines
    def shiftup50(self):
        if self.connected == False:
            self.throw_warning("warning", "connect to server first")
            return
        self.s.send('RUN2,1,0,0')
        self.recMSG = self.s.recv(self.MSGSIZE)
        data = self.handle_rm()#  LIST OF LINES ['2YR -1.93324 -10060.4 197.452', '5YR -29.5189 -103753 5819.57']
        risk_mv=[]
        two_hedge=[]
        print "data\n",data
        for item in data:
            item=item.split()[1:]
            print "item:",item
            risk_mv.append(item[0])
            risk_mv.append(item[1])
            two_hedge.append(item[2])
        print "risk_mv\n",risk_mv
        print "teo_hed\n",two_hedge
        self.c3.c3text1.delete('1.0','end')
        i = 0
        j = 0
        self.c3.c3text1.insert('end',"|Book\t\t|2YR\t\t\t\t|5YR\t\t\t\t|10YR\t\t\t\t|30YR\t\t\t\t|\n")
        self.c3.c3text1.insert('end',"|\t\t|Risk\t\t|MV\t\t|Risk\t\t|MV\t\t|Risk\t\t|MV\t\t|Risk\t\t|MV\t\t|\n")
        self.c3.c3text1.insert('end',"|Closing\t\t|"+risk_mv[i]+"\t\t|"+risk_mv[i+1]+"\t\t|"+risk_mv[i+2]+"\t\t|"+risk_mv[i+3]+"\t\t|"+risk_mv[i+4]+"\t\t|"+risk_mv[i+5]+"\t\t|"+risk_mv[i+6]+"\t\t|"+risk_mv[i+7]+"\t\t|\n")
        self.c3.c3text1.insert('end',"|2YR HEDGE\t\t|"+two_hedge[j]+"\t\t\t\t|"+two_hedge[j+1]+"\t\t\t\t|"+two_hedge[j+2]+"\t\t\t\t|"+two_hedge[j+3]+"\t\t\t\t|\n")
    def shiftdown50(self):
        if self.connected == False:
            self.throw_warning("warning", "connect to server first")
            return
        self.s.send('RUN2,1,1,0')
        self.recMSG = self.s.recv(self.MSGSIZE)
        data = self.handle_rm()#  LIST OF LINES ['2YR -1.93324 -10060.4 197.452', '5YR -29.5189 -103753 5819.57']
        risk_mv=[]
        two_hedge=[]
        print "data\n",data
        for item in data:
            item=item.split()[1:]
            print "item:",item
            risk_mv.append(item[0])
            risk_mv.append(item[1])
            two_hedge.append(item[2])
        print "risk_mv\n",risk_mv
        print "teo_hed\n",two_hedge
        self.c3.c3text1.delete('1.0','end')
        i = 0
        j = 0
        self.c3.c3text1.insert('end',"|Book\t\t|2YR\t\t\t\t|5YR\t\t\t\t|10YR\t\t\t\t|30YR\t\t\t\t|\n")
        self.c3.c3text1.insert('end',"|\t\t|Risk\t\t|MV\t\t|Risk\t\t|MV\t\t|Risk\t\t|MV\t\t|Risk\t\t|MV\t\t|\n")
        self.c3.c3text1.insert('end',"|Closing\t\t|"+risk_mv[i]+"\t\t|"+risk_mv[i+1]+"\t\t|"+risk_mv[i+2]+"\t\t|"+risk_mv[i+3]+"\t\t|"+risk_mv[i+4]+"\t\t|"+risk_mv[i+5]+"\t\t|"+risk_mv[i+6]+"\t\t|"+risk_mv[i+7]+"\t\t|\n")
        self.c3.c3text1.insert('end',"|2YR HEDGE\t\t|"+two_hedge[j]+"\t\t\t\t|"+two_hedge[j+1]+"\t\t\t\t|"+two_hedge[j+2]+"\t\t\t\t|"+two_hedge[j+3]+"\t\t\t\t|\n")
    def bp30(self,event):
        #s = int(s)
        bp = int(self.c3.lab1.get())
        '''if not isinstance(bp,numbers.Integral):
            self.throw_warning("Invalid Entry","Please Enter An Integer")
            return'''
        st=str('RUN2,2,3,')+str(bp)
        self.s.send(st)
        self.recMSG = self.s.recv(self.MSGSIZE)
        data = self.handle_rm()#  LIST OF LINES ['2YR -1.93324 -10060.4 197.452', '5YR -29.5189 -103753 5819.57']
        risk_mv=[]
        two_hedge=[]
        print "data\n",data
        for item in data:
            item=item.split()[1:]
            print "item:",item
            risk_mv.append(item[0])
            risk_mv.append(item[1])
            two_hedge.append(item[2])
        print "risk_mv\n",risk_mv
        print "teo_hed\n",two_hedge
        self.c3.c3text1.delete('1.0','end')
        i = 0
        j = 0
        self.c3.c3text1.insert('end',"|Book\t\t|2YR\t\t\t\t|5YR\t\t\t\t|10YR\t\t\t\t|30YR\t\t\t\t|\n")
        self.c3.c3text1.insert('end',"|\t\t|Risk\t\t|MV\t\t|Risk\t\t|MV\t\t|Risk\t\t|MV\t\t|Risk\t\t|MV\t\t|\n")
        self.c3.c3text1.insert('end',"|Closing\t\t|"+risk_mv[i]+"\t\t|"+risk_mv[i+1]+"\t\t|"+risk_mv[i+2]+"\t\t|"+risk_mv[i+3]+"\t\t|"+risk_mv[i+4]+"\t\t|"+risk_mv[i+5]+"\t\t|"+risk_mv[i+6]+"\t\t|"+risk_mv[i+7]+"\t\t|\n")
        self.c3.c3text1.insert('end',"|2YR HEDGE\t\t|"+two_hedge[j]+"\t\t\t\t|"+two_hedge[j+1]+"\t\t\t\t|"+two_hedge[j+2]+"\t\t\t\t|"+two_hedge[j+3]+"\t\t\t\t|\n")
    def bp10(self,event):
        #s = int(s)
        bp = int(self.c3.lab2.get())
        '''if not isinstance(bp,numbers.Integral):
            self.throw_warning("Invalid Entry","Please Enter An Integer")
            return'''
        st=str('RUN2,2,3,')+str(bp)
        self.s.send(st)
        self.recMSG = self.s.recv(self.MSGSIZE)
        data = self.handle_rm()#  LIST OF LINES ['2YR -1.93324 -10060.4 197.452', '5YR -29.5189 -103753 5819.57']
        risk_mv=[]
        two_hedge=[]
        print "data\n",data
        for item in data:
            item=item.split()[1:]
            print "item:",item
            risk_mv.append(item[0])
            risk_mv.append(item[1])
            two_hedge.append(item[2])
        print "risk_mv\n",risk_mv
        print "teo_hed\n",two_hedge
        self.c3.c3text1.delete('1.0','end')
        i = 0
        j = 0
        self.c3.c3text1.insert('end',"|Book\t\t|2YR\t\t\t\t|5YR\t\t\t\t|10YR\t\t\t\t|30YR\t\t\t\t|\n")
        self.c3.c3text1.insert('end',"|\t\t|Risk\t\t|MV\t\t|Risk\t\t|MV\t\t|Risk\t\t|MV\t\t|Risk\t\t|MV\t\t|\n")
        self.c3.c3text1.insert('end',"|Closing\t\t|"+risk_mv[i]+"\t\t|"+risk_mv[i+1]+"\t\t|"+risk_mv[i+2]+"\t\t|"+risk_mv[i+3]+"\t\t|"+risk_mv[i+4]+"\t\t|"+risk_mv[i+5]+"\t\t|"+risk_mv[i+6]+"\t\t|"+risk_mv[i+7]+"\t\t|\n")
        self.c3.c3text1.insert('end',"|2YR HEDGE\t\t|"+two_hedge[j]+"\t\t\t\t|"+two_hedge[j+1]+"\t\t\t\t|"+two_hedge[j+2]+"\t\t\t\t|"+two_hedge[j+3]+"\t\t\t\t|\n")
    def bp5(self,event):
        #s = int(s)
        bp = int(self.c3.lab3.get())
        '''if not isinstance(bp,numbers.Integral):
            self.throw_warning("Invalid Entry","Please Enter An Integer")
            return'''
        st=str('RUN2,2,3,')+str(bp)
        self.s.send(st)
        self.recMSG = self.s.recv(self.MSGSIZE)
        data = self.handle_rm()#  LIST OF LINES ['2YR -1.93324 -10060.4 197.452', '5YR -29.5189 -103753 5819.57']
        risk_mv=[]
        two_hedge=[]
        print "data\n",data
        for item in data:
            item=item.split()[1:]
            print "item:",item
            risk_mv.append(item[0])
            risk_mv.append(item[1])
            two_hedge.append(item[2])
        print "risk_mv\n",risk_mv
        print "teo_hed\n",two_hedge
        self.c3.c3text1.delete('1.0','end')
        i = 0
        j = 0
        self.c3.c3text1.insert('end',"|Book\t\t|2YR\t\t\t\t|5YR\t\t\t\t|10YR\t\t\t\t|30YR\t\t\t\t|\n")
        self.c3.c3text1.insert('end',"|\t\t|Risk\t\t|MV\t\t|Risk\t\t|MV\t\t|Risk\t\t|MV\t\t|Risk\t\t|MV\t\t|\n")
        self.c3.c3text1.insert('end',"|Closing\t\t|"+risk_mv[i]+"\t\t|"+risk_mv[i+1]+"\t\t|"+risk_mv[i+2]+"\t\t|"+risk_mv[i+3]+"\t\t|"+risk_mv[i+4]+"\t\t|"+risk_mv[i+5]+"\t\t|"+risk_mv[i+6]+"\t\t|"+risk_mv[i+7]+"\t\t|\n")
        self.c3.c3text1.insert('end',"|2YR HEDGE\t\t|"+two_hedge[j]+"\t\t\t\t|"+two_hedge[j+1]+"\t\t\t\t|"+two_hedge[j+2]+"\t\t\t\t|"+two_hedge[j+3]+"\t\t\t\t|\n")
    def bp2(self,event):
        #s = int(s)
        bp = int(self.c3.lab1.get())
        '''if not isinstance(bp,numbers.Integral):
            self.throw_warning("Invalid Entry","Please Enter An Integer")
            return'''
        st=str('RUN2,2,3,')+str(bp)
        self.s.send(st)
        self.recMSG = self.s.recv(self.MSGSIZE)
        data = self.handle_rm()#  LIST OF LINES ['2YR -1.93324 -10060.4 197.452', '5YR -29.5189 -103753 5819.57']
        risk_mv=[]
        two_hedge=[]
        print "data\n",data
        for item in data:
            item=item.split()[1:]
            print "item:",item
            risk_mv.append(item[0])
            risk_mv.append(item[1])
            two_hedge.append(item[2])
        print "risk_mv\n",risk_mv
        print "teo_hed\n",two_hedge
        self.c3.c3text1.delete('1.0','end')
        i = 0
        j = 0
        self.c3.c3text1.insert('end',"|Book\t\t|2YR\t\t\t\t|5YR\t\t\t\t|10YR\t\t\t\t|30YR\t\t\t\t|\n")
        self.c3.c3text1.insert('end',"|\t\t|Risk\t\t|MV\t\t|Risk\t\t|MV\t\t|Risk\t\t|MV\t\t|Risk\t\t|MV\t\t|\n")
        self.c3.c3text1.insert('end',"|Closing\t\t|"+risk_mv[i]+"\t\t|"+risk_mv[i+1]+"\t\t|"+risk_mv[i+2]+"\t\t|"+risk_mv[i+3]+"\t\t|"+risk_mv[i+4]+"\t\t|"+risk_mv[i+5]+"\t\t|"+risk_mv[i+6]+"\t\t|"+risk_mv[i+7]+"\t\t|\n")
        self.c3.c3text1.insert('end',"|2YR HEDGE\t\t|"+two_hedge[j]+"\t\t\t\t|"+two_hedge[j+1]+"\t\t\t\t|"+two_hedge[j+2]+"\t\t\t\t|"+two_hedge[j+3]+"\t\t\t\t|\n")

    def spawn_risk_by_maturity(self):
        if self.connected == False:
            self.throw_warning("warning", "connect to server first")
            return
        '''if self.risk_by_maturity_clean == True:
            self.throw_warning("warning", "Risk By Maturity window is already open")
            return'''
        #print "spawn_risk_by_maturity"
        self.risk_by_maturity_clean = True
        self.s.send('RUN2,0,0,0')
        self.recMSG = self.s.recv(self.MSGSIZE)
        data = self.handle_rm()#  LIST OF LINES ['2YR -1.93324 -10060.4 197.452', '5YR -29.5189 -103753 5819.57']
        risk_mv=[]
        two_hedge=[]
        print "data\n",data
        for item in data:
            item=item.split()[1:]
            print "item:",item
            risk_mv.append(item[0])
            risk_mv.append(item[1])
            two_hedge.append(item[2])
        print "risk_mv\n",risk_mv
        print "teo_hed\n",two_hedge


        self.c3 = tk.Toplevel()
        self.c3.geometry("1065x300")
        self.c3.protocol('WM_DELETE_WINDOW',self.cleanUpBeforeExit(2))

        self.c3.c3frame0 = tk.Frame(self.c3, bg='#EFEFEF', bd=4, width='1065', height='50', relief='sunken')
        self.c3.c3frame0.pack_propagate(False)
        self.c3.c3frame0.pack(side='top',padx=5,pady=5)
        self.c3.label01 = tk.Label(self.c3.c3frame0, fg = 'black', text="Risk by Maturity", font=self.headerFont)
        self.c3.label01.pack()

        self.c3.c3frame1 = tk.Frame(self.c3, bg='#DDDDDD', bd=2, height='100')
        self.c3.c3text1 = tk.Text(self.c3.c3frame1, bg='black', fg='#cccc00', bd=1, width='350', height='100', font=self.helv10)#, state='disabled')
        self.c3.c3frame1.pack_propagate(False)
        self.c3.c3text1.pack(expand='yes',fill='both')
        self.c3.c3frame1.pack(padx=20,pady=20,fill='x')#side='left'
        i = 0
        j = 0
        self.c3.c3text1.insert('end',"|Book\t\t|2YR\t\t\t\t|5YR\t\t\t\t|10YR\t\t\t\t|30YR\t\t\t\t|\n")
        self.c3.c3text1.insert('end',"|\t\t|Risk\t\t|MV\t\t|Risk\t\t|MV\t\t|Risk\t\t|MV\t\t|Risk\t\t|MV\t\t|\n")
        self.c3.c3text1.insert('end',"|Closing\t\t|"+risk_mv[i]+"\t\t|"+risk_mv[i+1]+"\t\t|"+risk_mv[i+2]+"\t\t|"+risk_mv[i+3]+"\t\t|"+risk_mv[i+4]+"\t\t|"+risk_mv[i+5]+"\t\t|"+risk_mv[i+6]+"\t\t|"+risk_mv[i+7]+"\t\t|\n")
        self.c3.c3text1.insert('end',"|2YR HEDGE\t\t|"+two_hedge[j]+"\t\t\t\t|"+two_hedge[j+1]+"\t\t\t\t|"+two_hedge[j+2]+"\t\t\t\t|"+two_hedge[j+3]+"\t\t\t\t|\n")

        self.c3.c3frame2 = tk.Frame(self.c3, bg='#EFEFEF', bd=4, width='1065', height='50')
        #self.c3.c3frame2.pack_propagate(False)
        self.c3.c3frame2.pack(side='bottom',padx=5,pady=5)
        

        self.c3.brsm1 = tk.Button(self.c3.c3frame2, text = "SHIFT UP 50%", fg = 'white', command = self.shiftup50, bg='#222222', relief='raised')
        self.c3.brsm1['font'] = self.helv10
        self.c3.brsm1.pack(side = 'left', padx=5,pady=20)

        self.c3.brsm2 = tk.Button(self.c3.c3frame2, text = "SHIFT DOWN 50%", fg = 'white', command = self.shiftdown50, bg='#222222', relief='raised')
        self.c3.brsm2['font'] = self.helv10
        self.c3.brsm2.pack(side = 'left', padx=5,pady=20)

        self.c3.lab1 = tk.Entry(self.c3.c3frame2)
        self.c3.lab1.pack(side='left')
        self.c3.lab1.insert(0,"50bp shift in 30YR")
        self.c3.lab1.bind("<Return>",self.bp30)

        return



        self.c3.c3text1.insert('end',self.recMSG)
    def spawn_histogram(self):
        if self.connected == False:
            self.throw_warning("warning", "connect to server first")
            return
        if self.histogram_clean == True:
            self.throw_warning("warning", "Histogram window is already open")
            return
        self.histogram_clean = True
        self.s.send('RUN3')
        self.recMSG = self.s.recv(self.MSGSIZE)
        print self.recMSG
        data = self.grab_hist_data()
        data1 = [("Bucket1",data[0]),("Bucket2",data[1]),("Bucket3",data[2]),("Bucket4",data[3]),("Bucket5",data[4]),("Bucket6",data[5]),("Bucket7",data[6]),("Bucket8",data[7])]
        N = len(data)
        x = np.arange(1,N+1)
        y = [num for (s,num) in data1]
        labels = [s for (s,num) in data1]
        width = 1

        bar1 = plt.bar(x, y, width, color = 'y')
        plt.ylabel('Frequency')
        plt.xticks(x + width/2.0, labels)
        plt.suptitle('Histogram')
        plt.show()
        return

    def setupLayout(self):
        ###########################  Frame 1  ######################################
        self.frame1 = tk.Frame(self, bg='#DDDDDD', bd=2)

        '''self.button1_tbSummary = tk.Button(self.frame1, text = "Trading Book Summary", command = self.tbSummary2, bg='#DDFFDD', relief='raised')
        self.button1_tbSummary['font'] = self.helv10
        self.button1_tbSummary.pack(side='left',padx=5,pady=20)'''

        self.frame1.pack(padx=20,pady=20,fill='x')
        ############ BUTTON - FRAME 1 FOR 1ST GUI SCREEN - SUMMARY ##################
        self.button1_daily_change = tk.Button(self.frame1, text = "DAILY CHANGE BY ISSUER", fg='white',command = self.spawn_daily_change, bg='#123956', relief='raised')
        self.button1_daily_change['font'] = self.helv10
        self.button1_daily_change.pack(side='left',padx=10,pady=20)
        ############ BUTTON - FRAME 1 FOR 2ND GUI SCREEN - RISK BY MATURITY : CLIENT SENDS "RUN2", CLIENT HANDLER: spawn_risk_by_maturity ##################
        self.button1_risk_by_maturity = tk.Button(self.frame1, text = "RISK BY MATURITY", fg='white',command = self.spawn_risk_by_maturity, bg='#123956', relief='raised')
        self.button1_risk_by_maturity['font'] = self.helv10
        self.button1_risk_by_maturity.pack(side='left',padx=10,pady=20)
        ############ BUTTON - FRAME 1 FOR 3RD GUI SCREEN - HISTOGRAM : CLIENT SENDS "RUN3", CLIENT HANDLER: spawn_histogram ##################
        self.button1_histogram = tk.Button(self.frame1, text = "HISTOGRAM", command = self.spawn_histogram, fg='white', bg='#123956', relief='raised')
        self.button1_histogram['font'] = self.helv10
        self.button1_histogram.pack(side='left',padx=10,pady=20)


        ###########################             Frame 2         #################################
        '''self.frame2 = tk.Frame(self, bg='#DDDDDD', bd=2)
        

        self.text2_serverOut = tk.Text(self.frame2, bg='white', bd=1)#, state='disabled')
        self.text2_serverOut.pack()
        self.text2_serverOut.insert('end',"Server Response:\n")
        self.frame2.pack(padx=20,pady=20,fill='x')'''
        ### Status Area ###
        self.status = tk.Label(self, text="All modules loaded...", bd=1, relief='sunken', anchor='w')
        self.status.pack(fill='x', side='bottom')
    #def setupGUI(self):
        #SOME FONTS


def main():
    app1 = mainApp()
    app1.mainloop()

main()

'''
HOST = 'MacBook-Pro.local'
PORT = 1234
MSGSIZE = 8192

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect((HOST, PORT))

connected = True

while connected == True:
        msg=''
        msg = raw_input("input msg 'EXIT' to disconnect, 'RUN' to run calculations: ")
        s.send(msg)
        data = s.recv(MSGSIZE)
        print 'Received from server: \n', data
        if(msg == 'EXIT'):
                connected = False
                s.close()
'''
