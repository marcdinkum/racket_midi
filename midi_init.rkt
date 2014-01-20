#lang racket

(require "midi_extension.rkt")

(list-devices)
(set-input-device 3)
(set-output-device 4)
(init-midi-out)

(define (read-loop)
  (let loop ((event (read-midi-event)))
    (when (list? event)
      (begin
        (displayln (caddr event))
        (note-on 0 (caddr event) 100)))
    (sleep 0.01)
    (loop (read-midi-event))))

; wat kunnen we hier nou mee?
; MIDI input EN output... dus filteren zou ik zeggen



