
import matplotlib
import matplotlib.pyplot as plt
import numpy as np
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
import PySimpleGUI as sg

# set the theme for the screen/window
sg.theme("DarkTanBlue")
# define layout
options = [[sg.Frame('Channels', [[sg.Radio('Ch1', 'rd_channels', key='Ch1'),
                                            sg.Radio('Ch2', 'rd_channels', key='Ch2'),
                                            sg.Radio('Ch2', 'rd_channels', key='Ch3'),
                                            sg.Radio('Ch4', 'rd_channels', key='Ch4')]],title_color='yellow',
                     border_width=10)],
           [sg.Frame('Triggers', [[sg.Radio('1', 'rd_triggers', key='1'),
                                   sg.Radio('2', 'rd_triggers', key='2'),
                                   sg.Radio('3', 'rd_triggers', key='3'),
                                   sg.Radio('4', 'rd_triggers', key='4')]],title_color='yellow',
           border_width = 10)],

           [sg.Frame('Vertical', [[sg.Checkbox('Onion', key='Onion Sauce'),
                                             sg.Checkbox('Paprika', key='Paprika'),
                                             sg.Checkbox('Schezwan', key='Schezwan'),
                                             sg.Checkbox('Tandoori', key='Tandoori')]], title_color='yellow',
                     border_width=10)],
[sg.Frame('Functions', [[sg.Checkbox('Peak-To-Peak', key='Peak-To-Peak'),
                                             sg.Checkbox('Max', key='Max'),
                                             sg.Checkbox('Min', key='Min'),
                                             sg.Checkbox('Frequency', key='Frequency'),
                                             sg.Checkbox('RMS', key='RMS')]], title_color='yellow',
                     border_width=10)],
           [sg.Button('Start', font=('Times New Roman', 12)),
           sg.Button('Close', font=('Times New Roman', 12))]]
choices = [[sg.Frame('Oscilloscope 4Channels', layout=options)]]


#---------------------random Sin wave Plot for demo-Alex's code will go here-----------------

fig = matplotlib.figure.Figure(figsize=(5, 4), dpi=100)
t = np.arange(0, 3, .01)
fig.add_subplot(111).plot(t, 2 * np.sin(2 * np.pi * t))



# ------------------------------- Beginning of Matplotlib helper code -----------------------

def draw_figure(canvas, figure):
    figure_canvas_agg = FigureCanvasTkAgg(figure, canvas)
    figure_canvas_agg.draw()
    figure_canvas_agg.get_tk_widget().pack(side='top', fill='both', expand=1)
    return figure_canvas_agg

# ------------------------------- Beginning of GUI CODE -------------------------------

# Create layout with two columns using precreated frames
layout = [[sg.Column(choices), (sg.Canvas(key='-CANVAS-'))]]

# create the form and show it without the plot
window = sg.Window('Demo Application - 4 Channel Oscilloscope', layout, location=(100,100), finalize=True)

# add the plot to the window
fig_canvas_agg = draw_figure(window['-CANVAS-'].TKCanvas, fig)

# Read  values entered by user
event, values = window.read()
# access all the values and if selected add them to a string
strx = ""
for val in values:
    if window.FindElement(val).get() == True:
        strx = strx + " " + val + ","

while True:
    event, values = window.read()  # Read  values entered by user
    if event == sg.WIN_CLOSED:  # If window is closed by user terminate While Loop
        break
    if event == 'Start':  # If submit button is clicked display chosen values
        window['options'].update(strx)  # output the final string
# Close Window
window.close()
