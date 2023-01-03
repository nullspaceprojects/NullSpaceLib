/*
  NullSpaceLib.h - Library for Null Space Projects YouTube.
  Created by Null Space Projects, November 21, 2022.
  Released into the public domain.
*/


#ifndef NullSpaceLib_h
#define NullSpaceLib_h

#include "Arduino.h"

/********* DEFINIZIONE CLASSE HELPER TIMER ************/
class TimerC {

  private:
        unsigned long _start_time;
        unsigned long _elapsed_time;
  
  public:
      TimerC()
      {
        _start_time = 0;
        _elapsed_time = 0;
      }

      void start()
      {
        if(_start_time != 0)
        {
          return;
        }
        _start_time = millis();
      }
      
      unsigned long getET()
      {
        //in millisecondi
        _elapsed_time = millis() - _start_time;
        return _elapsed_time;
      }
      double getETSec()
      {
        unsigned long et_milli =  getET();
        return et_milli/1000.0;
      }
      void reset()
      {
        _start_time = millis();
        _elapsed_time = 0;
      }
      void stop_()
      {
        if(_start_time == 0)
        {
          return;
        }
        _start_time = 0;
        _elapsed_time = 0;
        
      }
};
class TimerMicro {

  private:
        unsigned long _start_time;
        unsigned long _elapsed_time;
  
  public:
      TimerMicro()
      {
        _start_time = 0;
        _elapsed_time = 0;
      }

      void start()
      {
        if(_start_time != 0)
        {
          return;
        }
        _start_time = micros();
      }
      
      unsigned long getET()
      {
        //in microsec
        _elapsed_time = micros() - _start_time;
        return _elapsed_time;
      }
      double getETMillis()
      {
        unsigned long et_micro =  getET();
        return et_micro/1000.0;
      }
	  double getETSec()
      {
        return getETMillis()/1000.0;
        
      }
      void reset()
      {
        _start_time = micros();
        _elapsed_time = 0;
      }
      void stop_()
      {
        if(_start_time == 0)
        {
          return;
        }
        _start_time = 0;
        _elapsed_time = 0;
        
      }
};


class LPFClass
{
	public:
	 float valfilt;
	 float alpha;
	 
	 LPFClass(float alpha_)
	 {
		 alpha = alpha_;
		 valfilt=0.0f;
	 }
	 float compute(float in_val)
	 {
		 //LPF
		 //filtered_output[i] = α*raw_input[i] + (1-α)*filtered_output[i-1]
		 valfilt = alpha*in_val + (1.f - alpha)*valfilt;
		 return valfilt;
	 }
};

class R_TRIG
{
    private:
      bool state;
      bool state_old;  
      bool q;
    public:
      bool getQ()
      {
          return q;
      }
  
      R_TRIG(bool init)
      {
          q = false;
          state = init;
          state_old = init;
      }
      bool call(bool _in)
      {
          state = _in;
          if (state && !state_old)
          {
              //fronte di salita
              q = true;
          }
          else
          {
              q = false;
          }
          state_old = state;
          return q;
      }
};

class Debouncer
{
	private:
		bool buttonState;             // the current reading from the input pin
		bool lastButtonState;   // the previous reading from the input pin
		uint64_t lastDebounceTime = 0;  // the last time the output pin was toggled
		uint64_t debounceDelay;    //[ms] the debounce time; increase if the output flickers
	public:
		Debouncer(bool ButtonStateInit=false, unsigned long delay_ms=50)
		{
			lastButtonState = ButtonStateInit;
			buttonState = ButtonStateInit;
			debounceDelay = delay_ms;
		}
		bool run(int reading)
		{
			// check to see if you just pressed the button
			// (i.e. the input went from LOW to HIGH), and you've waited long enough
			// since the last press to ignore any noise:

			// If the switch changed, due to noise or pressing:
			if (reading != lastButtonState) {
				// reset the debouncing timer
				lastDebounceTime = millis();
			}
			if ((millis() - lastDebounceTime) > debounceDelay) 
			{
				// whatever the reading is at, it's been there for longer than the debounce
				// delay, so take it as the actual current state:

				// if the button state has changed:
				if (reading != buttonState) 
				{
					buttonState = reading;
				}
  			}
			// save the reading. Next time through the loop, it'll be the lastButtonState:
  			lastButtonState = reading;
			return buttonState;
		}
		
		  
};

class TON 
{
	private:
		TimerC* _timer;
		R_TRIG* _fs, *_fd;
	public:
		bool q;
		uint64_t pt_ms;

	public:
		TON(bool in_=false, uint64_t pt_ms_ = 50)
		{
			pt_ms=pt_ms_;
			_timer = new TimerC();
			_fs = new R_TRIG(in_);
			_fd = new R_TRIG(in_);
			_timer->start();
		}
		bool run(bool in_)
		{
			if ( _fs->call(in_) )
			{
				//sul fronte di salita del ingresso
				_timer->reset();
			}

			if ( _fd->call(in_==false) )
			{
				//sul fronte di discesa del ingresso
				_timer->reset();
			}

			if (in_ && _timer->getET()>=pt_ms)
			{
				q=true;
			}
			else
			{
				q=false;
			}
			return q;
			
		}

		~TON()
		{
			delete _timer;
			delete _fs;
			delete _fd;
		}
};

#define MEDIAN_FILTER_WINDOW_SIZE (int)7
class MedianFilterClass
{
	private:
		int num_elementi_inseriti;
		float window_copy[MEDIAN_FILTER_WINDOW_SIZE];
	public:
		float valfilt;
		float window[MEDIAN_FILTER_WINDOW_SIZE];
		

	MedianFilterClass()
	{
		 for(int i=0;i<MEDIAN_FILTER_WINDOW_SIZE;++i)
		 {
			 window[i]=0;
			 window_copy[i]=0;
		 }
		 valfilt=0.0f;
		 num_elementi_inseriti =0;
	}
	float compute(float in_val)
	{
		this->left_shift_window(in_val);
		if (num_elementi_inseriti<=MEDIAN_FILTER_WINDOW_SIZE)
		{
			++num_elementi_inseriti;
			this->valfilt=in_val;
			return this->valfilt;
		}
		
		//copy window to window_copy
		 for(int i=0;i<MEDIAN_FILTER_WINDOW_SIZE;++i)
		 {
			 window_copy[i]=window[i];
		 }
		
		//da qui la window è piena con tutti elementi di velocita
		//Order elements (only half of them)
		int middle_idx=MEDIAN_FILTER_WINDOW_SIZE/2;
		for (int j = 0; j <= middle_idx; ++j)
		{
		 //   Find position of minimum element
		 int min = j;
		 for (int k = j + 1; k < MEDIAN_FILTER_WINDOW_SIZE; ++k)
			if (window_copy[k] < window_copy[min])
			   min = k;
		 //   Put found minimum element in its place
		 const float temp = window_copy[j];
		 window_copy[j] = window_copy[min];
		 window_copy[min] = temp;
		}
		this->valfilt = window_copy[middle_idx];
		return this->valfilt;
		
	}
	private:
		void left_shift_window(float new_elem)
		{
			//butta via il primo elemento a sinistra
			for(int i=0;i<MEDIAN_FILTER_WINDOW_SIZE-1;++i)
			{
				window[i]=window[i+1];
			}
			window[MEDIAN_FILTER_WINDOW_SIZE-1] = new_elem;
		}
		
};



template <class T>
class Point
{
  public:
    T x;
    T y;
    T z;
    T vx;
    T vy;
    T vz;
    Point()
    {
      this->Reset();
    }    
    Point(T x_, T y_, T z_)
    {
      x=x_;
      y=y_;
      z=z_;
      vx=0;
      vy=0;
      vz=0;
    }
    void nextStep()
    {
      x+=vx;
      y+=vy;
      z+=vz;
    }
    void Reset()
    {
      x=0;
      y=0;
      z=0;
      vx=0;
      vy=0;
      vz=0;
    }
};



class Cube
{
  public:
    Point<float> vert[8];
  public:
    Cube()
    {
      vert[0].x = -1.000000;vert[0].y = 1.000000;vert[0].z = 1.000000;
      vert[1].x = -1.000000;vert[1].y = 1.000000;vert[1].z = -1.000000;
      vert[2].x = 1.000000;vert[2].y = 1.000000;vert[2].z = 1.000000;
      vert[3].x = 1.000000;vert[3].y = 1.000000;vert[3].z = -1.000000;
      vert[4].x = -1.000000;vert[4].y = -1.000000;vert[4].z = 1.000000;
      vert[5].x = -1.000000;vert[5].y = -1.000000;vert[5].z = -1.000000;
      vert[6].x = 1.000000;vert[6].y = -1.000000;vert[6].z = 1.000000;
      vert[7].x = 1.000000;vert[7].y = -1.000000;vert[7].z = -1.000000;
    }
    void Scale(float scale)
    {
        for (int t=0; t<8; ++t) {
          vert[t].x*=scale;
          vert[t].y*=scale;
          vert[t].z*=scale;
        }
    }
    
};

#define N_VERT_Cube1 26
#define N_FACE_Cube1 48
class Cube1
{
  public:

	Point<float> vert[N_VERT_Cube1];
	Point<int> faces[N_FACE_Cube1];
  public:
    Cube1()
    {   
vert[0].x = 1.000000;vert[0].y = -1.000000;vert[0].z = 1.000000;
vert[1].x = 1.000000;vert[1].y = -1.000000;vert[1].z = -1.000000;
vert[2].x = 1.000000;vert[2].y = 1.000000;vert[2].z = -1.000000;
vert[3].x = 1.000000;vert[3].y = 1.000000;vert[3].z = 1.000000;
vert[4].x = -1.000000;vert[4].y = 1.000000;vert[4].z = 1.000000;
vert[5].x = -1.000000;vert[5].y = 1.000000;vert[5].z = -1.000000;
vert[6].x = -1.000000;vert[6].y = -1.000000;vert[6].z = -1.000000;
vert[7].x = -1.000000;vert[7].y = -1.000000;vert[7].z = 1.000000;
vert[8].x = 1.000000;vert[8].y = 0.000000;vert[8].z = 1.000000;
vert[9].x = 1.000000;vert[9].y = -1.000000;vert[9].z = -0.000000;
vert[10].x = 1.000000;vert[10].y = 0.000000;vert[10].z = -1.000000;
vert[11].x = 1.000000;vert[11].y = 1.000000;vert[11].z = -0.000000;
vert[12].x = -1.000000;vert[12].y = 1.000000;vert[12].z = 0.000000;
vert[13].x = 0.000000;vert[13].y = 1.000000;vert[13].z = 1.000000;
vert[14].x = -0.000000;vert[14].y = 1.000000;vert[14].z = -1.000000;
vert[15].x = -1.000000;vert[15].y = 0.000000;vert[15].z = -1.000000;
vert[16].x = -0.000000;vert[16].y = -1.000000;vert[16].z = -1.000000;
vert[17].x = -1.000000;vert[17].y = -1.000000;vert[17].z = 0.000000;
vert[18].x = -1.000000;vert[18].y = 0.000000;vert[18].z = 1.000000;
vert[19].x = 0.000000;vert[19].y = -1.000000;vert[19].z = 1.000000;
vert[20].x = 0.000000;vert[20].y = -1.000000;vert[20].z = 0.000000;
vert[21].x = 0.000000;vert[21].y = 0.000000;vert[21].z = 1.000000;
vert[22].x = -1.000000;vert[22].y = 0.000000;vert[22].z = 0.000000;
vert[23].x = -0.000000;vert[23].y = 0.000000;vert[23].z = -1.000000;
vert[24].x = 0.000000;vert[24].y = 1.000000;vert[24].z = 0.000000;
vert[25].x = 1.000000;vert[25].y = 0.000000;vert[25].z = -0.000000;
faces[0].x = 10;faces[0].y = 11;faces[0].z = 25;
faces[1].x = 11;faces[1].y = 14;faces[1].z = 24;
faces[2].x = 10;faces[2].y = 16;faces[2].z = 23;
faces[3].x = 18;faces[3].y = 12;faces[3].z = 22;
faces[4].x = 8;faces[4].y = 13;faces[4].z = 21;
faces[5].x = 9;faces[5].y = 19;faces[5].z = 20;
faces[6].x = 20;faces[6].y = 7;faces[6].z = 17;
faces[7].x = 16;faces[7].y = 17;faces[7].z = 6;
faces[8].x = 1;faces[8].y = 20;faces[8].z = 16;
faces[9].x = 21;faces[9].y = 4;faces[9].z = 18;
faces[10].x = 19;faces[10].y = 18;faces[10].z = 7;
faces[11].x = 0;faces[11].y = 21;faces[11].z = 19;
faces[12].x = 22;faces[12].y = 5;faces[12].z = 15;
faces[13].x = 17;faces[13].y = 15;faces[13].z = 6;
faces[14].x = 7;faces[14].y = 22;faces[14].z = 17;
faces[15].x = 23;faces[15].y = 6;faces[15].z = 15;
faces[16].x = 14;faces[16].y = 15;faces[16].z = 5;
faces[17].x = 2;faces[17].y = 23;faces[17].z = 14;
faces[18].x = 24;faces[18].y = 5;faces[18].z = 12;
faces[19].x = 13;faces[19].y = 12;faces[19].z = 4;
faces[20].x = 3;faces[20].y = 24;faces[20].z = 13;
faces[21].x = 25;faces[21].y = 3;faces[21].z = 8;
faces[22].x = 9;faces[22].y = 8;faces[22].z = 0;
faces[23].x = 1;faces[23].y = 25;faces[23].z = 9;
faces[24].x = 10;faces[24].y = 2;faces[24].z = 11;
faces[25].x = 11;faces[25].y = 2;faces[25].z = 14;
faces[26].x = 10;faces[26].y = 1;faces[26].z = 16;
faces[27].x = 18;faces[27].y = 4;faces[27].z = 12;
faces[28].x = 8;faces[28].y = 3;faces[28].z = 13;
faces[29].x = 9;faces[29].y = 0;faces[29].z = 19;
faces[30].x = 20;faces[30].y = 19;faces[30].z = 7;
faces[31].x = 16;faces[31].y = 20;faces[31].z = 17;
faces[32].x = 1;faces[32].y = 9;faces[32].z = 20;
faces[33].x = 21;faces[33].y = 13;faces[33].z = 4;
faces[34].x = 19;faces[34].y = 21;faces[34].z = 18;
faces[35].x = 0;faces[35].y = 8;faces[35].z = 21;
faces[36].x = 22;faces[36].y = 12;faces[36].z = 5;
faces[37].x = 17;faces[37].y = 22;faces[37].z = 15;
faces[38].x = 7;faces[38].y = 18;faces[38].z = 22;
faces[39].x = 23;faces[39].y = 16;faces[39].z = 6;
faces[40].x = 14;faces[40].y = 23;faces[40].z = 15;
faces[41].x = 2;faces[41].y = 10;faces[41].z = 23;
faces[42].x = 24;faces[42].y = 14;faces[42].z = 5;
faces[43].x = 13;faces[43].y = 24;faces[43].z = 12;
faces[44].x = 3;faces[44].y = 11;faces[44].z = 24;
faces[45].x = 25;faces[45].y = 11;faces[45].z = 3;
faces[46].x = 9;faces[46].y = 25;faces[46].z = 8;
faces[47].x = 1;faces[47].y = 10;faces[47].z = 25;



    }
    void Scale(float scale)
    {
        for (int t=0; t<N_VERT_Cube1; ++t) {
          vert[t].x*=scale;
          vert[t].y*=scale;
          vert[t].z*=scale;
        }
    }
    
};
#define N_VERT_Icospere 42
#define N_FACE_Icospere 80
class Icospere
{
  public:

	Point<float> vert[N_VERT_Icospere];
	Point<int> faces[N_FACE_Icospere];
  public:
    Icospere()
    {   
vert[0].x = 0.000000;vert[0].y = 0.000000;vert[0].z = -1.000000;
vert[1].x = 0.525725;vert[1].y = 0.723607;vert[1].z = -0.447220;
vert[2].x = 0.850649;vert[2].y = -0.276388;vert[2].z = -0.447220;
vert[3].x = 0.000000;vert[3].y = -0.894426;vert[3].z = -0.447216;
vert[4].x = -0.850649;vert[4].y = -0.276388;vert[4].z = -0.447220;
vert[5].x = -0.525725;vert[5].y = 0.723607;vert[5].z = -0.447220;
vert[6].x = 0.850649;vert[6].y = 0.276388;vert[6].z = 0.447220;
vert[7].x = 0.525725;vert[7].y = -0.723607;vert[7].z = 0.447220;
vert[8].x = -0.525725;vert[8].y = -0.723607;vert[8].z = 0.447220;
vert[9].x = -0.850649;vert[9].y = 0.276388;vert[9].z = 0.447220;
vert[10].x = 0.000000;vert[10].y = 0.894426;vert[10].z = 0.447216;
vert[11].x = 0.000000;vert[11].y = 0.000000;vert[11].z = 1.000000;
vert[12].x = 0.499995;vert[12].y = -0.162456;vert[12].z = -0.850654;
vert[13].x = 0.309011;vert[13].y = 0.425323;vert[13].z = -0.850654;
vert[14].x = 0.809012;vert[14].y = 0.262869;vert[14].z = -0.525738;
vert[15].x = 0.000000;vert[15].y = 0.850648;vert[15].z = -0.525736;
vert[16].x = -0.309011;vert[16].y = 0.425323;vert[16].z = -0.850654;
vert[17].x = 0.000000;vert[17].y = -0.525730;vert[17].z = -0.850652;
vert[18].x = 0.499997;vert[18].y = -0.688189;vert[18].z = -0.525736;
vert[19].x = -0.499995;vert[19].y = -0.162456;vert[19].z = -0.850654;
vert[20].x = -0.499997;vert[20].y = -0.688189;vert[20].z = -0.525736;
vert[21].x = -0.809012;vert[21].y = 0.262869;vert[21].z = -0.525738;
vert[22].x = 0.309013;vert[22].y = 0.951058;vert[22].z = 0.000000;
vert[23].x = -0.309013;vert[23].y = 0.951058;vert[23].z = 0.000000;
vert[24].x = 1.000000;vert[24].y = 0.000000;vert[24].z = 0.000000;
vert[25].x = 0.809017;vert[25].y = 0.587786;vert[25].z = 0.000000;
vert[26].x = 0.309013;vert[26].y = -0.951058;vert[26].z = 0.000000;
vert[27].x = 0.809017;vert[27].y = -0.587786;vert[27].z = 0.000000;
vert[28].x = -0.809017;vert[28].y = -0.587786;vert[28].z = 0.000000;
vert[29].x = -0.309013;vert[29].y = -0.951058;vert[29].z = 0.000000;
vert[30].x = -0.809017;vert[30].y = 0.587786;vert[30].z = 0.000000;
vert[31].x = -1.000000;vert[31].y = 0.000000;vert[31].z = 0.000000;
vert[32].x = 0.499997;vert[32].y = 0.688189;vert[32].z = 0.525736;
vert[33].x = 0.809012;vert[33].y = -0.262869;vert[33].z = 0.525738;
vert[34].x = 0.000000;vert[34].y = -0.850648;vert[34].z = 0.525736;
vert[35].x = -0.809012;vert[35].y = -0.262869;vert[35].z = 0.525738;
vert[36].x = -0.499997;vert[36].y = 0.688189;vert[36].z = 0.525736;
vert[37].x = 0.499995;vert[37].y = 0.162456;vert[37].z = 0.850654;
vert[38].x = 0.000000;vert[38].y = 0.525730;vert[38].z = 0.850652;
vert[39].x = 0.309011;vert[39].y = -0.425323;vert[39].z = 0.850654;
vert[40].x = -0.309011;vert[40].y = -0.425323;vert[40].z = 0.850654;
vert[41].x = -0.499995;vert[41].y = 0.162456;vert[41].z = 0.850654;
faces[0].x = 0;faces[0].y = 13;faces[0].z = 12;
faces[1].x = 1;faces[1].y = 13;faces[1].z = 15;
faces[2].x = 0;faces[2].y = 12;faces[2].z = 17;
faces[3].x = 0;faces[3].y = 17;faces[3].z = 19;
faces[4].x = 0;faces[4].y = 19;faces[4].z = 16;
faces[5].x = 1;faces[5].y = 15;faces[5].z = 22;
faces[6].x = 2;faces[6].y = 14;faces[6].z = 24;
faces[7].x = 3;faces[7].y = 18;faces[7].z = 26;
faces[8].x = 4;faces[8].y = 20;faces[8].z = 28;
faces[9].x = 5;faces[9].y = 21;faces[9].z = 30;
faces[10].x = 1;faces[10].y = 22;faces[10].z = 25;
faces[11].x = 2;faces[11].y = 24;faces[11].z = 27;
faces[12].x = 3;faces[12].y = 26;faces[12].z = 29;
faces[13].x = 4;faces[13].y = 28;faces[13].z = 31;
faces[14].x = 5;faces[14].y = 30;faces[14].z = 23;
faces[15].x = 6;faces[15].y = 32;faces[15].z = 37;
faces[16].x = 7;faces[16].y = 33;faces[16].z = 39;
faces[17].x = 8;faces[17].y = 34;faces[17].z = 40;
faces[18].x = 9;faces[18].y = 35;faces[18].z = 41;
faces[19].x = 10;faces[19].y = 36;faces[19].z = 38;
faces[20].x = 38;faces[20].y = 41;faces[20].z = 11;
faces[21].x = 38;faces[21].y = 36;faces[21].z = 41;
faces[22].x = 36;faces[22].y = 9;faces[22].z = 41;
faces[23].x = 41;faces[23].y = 40;faces[23].z = 11;
faces[24].x = 41;faces[24].y = 35;faces[24].z = 40;
faces[25].x = 35;faces[25].y = 8;faces[25].z = 40;
faces[26].x = 40;faces[26].y = 39;faces[26].z = 11;
faces[27].x = 40;faces[27].y = 34;faces[27].z = 39;
faces[28].x = 34;faces[28].y = 7;faces[28].z = 39;
faces[29].x = 39;faces[29].y = 37;faces[29].z = 11;
faces[30].x = 39;faces[30].y = 33;faces[30].z = 37;
faces[31].x = 33;faces[31].y = 6;faces[31].z = 37;
faces[32].x = 37;faces[32].y = 38;faces[32].z = 11;
faces[33].x = 37;faces[33].y = 32;faces[33].z = 38;
faces[34].x = 32;faces[34].y = 10;faces[34].z = 38;
faces[35].x = 23;faces[35].y = 36;faces[35].z = 10;
faces[36].x = 23;faces[36].y = 30;faces[36].z = 36;
faces[37].x = 30;faces[37].y = 9;faces[37].z = 36;
faces[38].x = 31;faces[38].y = 35;faces[38].z = 9;
faces[39].x = 31;faces[39].y = 28;faces[39].z = 35;
faces[40].x = 28;faces[40].y = 8;faces[40].z = 35;
faces[41].x = 29;faces[41].y = 34;faces[41].z = 8;
faces[42].x = 29;faces[42].y = 26;faces[42].z = 34;
faces[43].x = 26;faces[43].y = 7;faces[43].z = 34;
faces[44].x = 27;faces[44].y = 33;faces[44].z = 7;
faces[45].x = 27;faces[45].y = 24;faces[45].z = 33;
faces[46].x = 24;faces[46].y = 6;faces[46].z = 33;
faces[47].x = 25;faces[47].y = 32;faces[47].z = 6;
faces[48].x = 25;faces[48].y = 22;faces[48].z = 32;
faces[49].x = 22;faces[49].y = 10;faces[49].z = 32;
faces[50].x = 30;faces[50].y = 31;faces[50].z = 9;
faces[51].x = 30;faces[51].y = 21;faces[51].z = 31;
faces[52].x = 21;faces[52].y = 4;faces[52].z = 31;
faces[53].x = 28;faces[53].y = 29;faces[53].z = 8;
faces[54].x = 28;faces[54].y = 20;faces[54].z = 29;
faces[55].x = 20;faces[55].y = 3;faces[55].z = 29;
faces[56].x = 26;faces[56].y = 27;faces[56].z = 7;
faces[57].x = 26;faces[57].y = 18;faces[57].z = 27;
faces[58].x = 18;faces[58].y = 2;faces[58].z = 27;
faces[59].x = 24;faces[59].y = 25;faces[59].z = 6;
faces[60].x = 24;faces[60].y = 14;faces[60].z = 25;
faces[61].x = 14;faces[61].y = 1;faces[61].z = 25;
faces[62].x = 22;faces[62].y = 23;faces[62].z = 10;
faces[63].x = 22;faces[63].y = 15;faces[63].z = 23;
faces[64].x = 15;faces[64].y = 5;faces[64].z = 23;
faces[65].x = 16;faces[65].y = 21;faces[65].z = 5;
faces[66].x = 16;faces[66].y = 19;faces[66].z = 21;
faces[67].x = 19;faces[67].y = 4;faces[67].z = 21;
faces[68].x = 19;faces[68].y = 20;faces[68].z = 4;
faces[69].x = 19;faces[69].y = 17;faces[69].z = 20;
faces[70].x = 17;faces[70].y = 3;faces[70].z = 20;
faces[71].x = 17;faces[71].y = 18;faces[71].z = 3;
faces[72].x = 17;faces[72].y = 12;faces[72].z = 18;
faces[73].x = 12;faces[73].y = 2;faces[73].z = 18;
faces[74].x = 15;faces[74].y = 16;faces[74].z = 5;
faces[75].x = 15;faces[75].y = 13;faces[75].z = 16;
faces[76].x = 13;faces[76].y = 0;faces[76].z = 16;
faces[77].x = 12;faces[77].y = 14;faces[77].z = 2;
faces[78].x = 12;faces[78].y = 13;faces[78].z = 14;
faces[79].x = 13;faces[79].y = 1;faces[79].z = 14;

    }
    void Scale(float scale)
    {
        for (int t=0; t<N_VERT_Icospere; ++t) {
          vert[t].x*=scale;
          vert[t].y*=scale;
          vert[t].z*=scale;
        }
    }
    
};

class PinHoleCamera
{
  public:
    float fx;
    float fy;
    float cx;
    float cy;
    float f;
  private:
    float sx;
    float sy;
  public:
    PinHoleCamera()
    {
      cx=128/2.0;
      cy=64/2.0;
      sx=10;//128/34.0;
      sy=10;//64/23.0;
      f=6;
      fx=sx*f;
      fy=sy*f;
    }
    void setFocalLength(float f_)
    {
      this->f = f_;
    }
    void ProjectK(float& u, float& v, const Point<float>& vertex)
    {
      float w=vertex.z;
      u = (fx*vertex.x + cx*vertex.z)/w;
      v = (fy*vertex.y + cy*vertex.z)/w;
    }
    void RotoTraslation(float yawdeg, float pitchdeg, float rolldeg, float tx, float ty, float tz, const Point<float>& xyz_in, Point<float>& xyz_out )
    {
      float yaw = yawdeg*PI/180.0;
      float pitch = pitchdeg*PI/180.0;
      float roll = rolldeg*PI/180.0;

      float x=xyz_in.x;
      float y=xyz_in.y;
      float z=xyz_in.z;

      xyz_out.x = tx - y*(cos(roll)*sin(yaw) - cos(yaw)*sin(pitch)*sin(roll)) + z*(sin(roll)*sin(yaw) + cos(roll)*cos(yaw)*sin(pitch)) + x*cos(pitch)*cos(yaw);
      xyz_out.y = ty + y*(cos(roll)*cos(yaw) + sin(pitch)*sin(roll)*sin(yaw)) - z*(cos(yaw)*sin(roll) - cos(roll)*sin(pitch)*sin(yaw)) + x*cos(pitch)*sin(yaw);
      xyz_out.z = tz - x*sin(pitch) + z*cos(pitch)*cos(roll) + y*cos(pitch)*sin(roll);

      //Serial.println("xyz_in x: " +String(x,1)+ " xyz_in y: "+String(y,1)+ " xyz_in z: "+String(z,1));
      //Serial.println("xyz_out x: " +String(xyz_out.x,1)+ " xyz_out y: "+String(xyz_out.y,1)+ " xyz_out z: "+String(xyz_out.z,1));
    }
     
};

template <class T>
class Triangle
{
	public:
		Triangle(){}
		Point<T> v1;
		Point<T> v2;
		Point<T> v3;
		
};



#define N_VERT_Logo3DNSP 896
#define N_FACE_Logo3DNSP 892
class Logo3DNSP
{
  public:
    Point<float> vert[N_VERT_Logo3DNSP];
	Point<int> faces[N_FACE_Logo3DNSP];
  public:
    Logo3DNSP()
    {
	  vert[0].x = 0.000000;vert[0].y = 1.009163;vert[0].z = -0.250000;
vert[1].x = 0.000000;vert[1].y = 1.009163;vert[1].z = -0.250000;
vert[2].x = 0.000000;vert[2].y = 1.009163;vert[2].z = -0.250000;
vert[3].x = -0.261191;vert[3].y = 0.974777;vert[3].z = -0.250000;
vert[4].x = -0.344913;vert[4].y = 0.948298;vert[4].z = -0.250000;
vert[5].x = -0.249182;vert[5].y = 0.849270;vert[5].z = -0.250000;
vert[6].x = 0.000000;vert[6].y = 0.885081;vert[6].z = -0.250000;
vert[7].x = 0.249182;vert[7].y = 0.849270;vert[7].z = -0.250000;
vert[8].x = 0.344913;vert[8].y = 0.948298;vert[8].z = -0.250000;
vert[9].x = 0.344913;vert[9].y = 0.948298;vert[9].z = -0.250000;
vert[10].x = -0.504582;vert[10].y = 0.873961;vert[10].z = -0.250000;
vert[11].x = -0.576854;vert[11].y = 0.827936;vert[11].z = -0.250000;
vert[12].x = -0.405775;vert[12].y = 0.786546;vert[12].z = -0.250000;
vert[13].x = -0.168149;vert[13].y = 0.868926;vert[13].z = -0.250000;
vert[14].x = 0.000000;vert[14].y = 0.885081;vert[14].z = -0.250000;
vert[15].x = 0.000000;vert[15].y = 0.885081;vert[15].z = -0.250000;
vert[16].x = 0.168149;vert[16].y = 0.868926;vert[16].z = -0.250000;
vert[17].x = 0.311052;vert[17].y = 0.828532;vert[17].z = -0.250000;
vert[18].x = 0.405775;vert[18].y = 0.786546;vert[18].z = -0.250000;
vert[19].x = 0.504582;vert[19].y = 0.873961;vert[19].z = -0.250000;
vert[20].x = 0.576854;vert[20].y = 0.827936;vert[20].z = -0.250000;
vert[21].x = -0.695028;vert[21].y = 0.731655;vert[21].z = -0.250000;
vert[22].x = -0.579746;vert[22].y = 0.668697;vert[22].z = -0.250000;
vert[23].x = -0.405775;vert[23].y = 0.786546;vert[23].z = -0.250000;
vert[24].x = -0.758495;vert[24].y = 0.665560;vert[24].z = -0.250000;
vert[25].x = -0.616872;vert[25].y = 0.634690;vert[25].z = -0.250000;
vert[26].x = -0.839195;vert[26].y = 0.560390;vert[26].z = -0.250000;
vert[27].x = -0.744685;vert[27].y = 0.478281;vert[27].z = -0.250000;
vert[28].x = -0.904910;vert[28].y = 0.446569;vert[28].z = -0.250000;
vert[29].x = -0.744685;vert[29].y = 0.478281;vert[29].z = -0.250000;
vert[30].x = -0.815093;vert[30].y = 0.344706;vert[30].z = -0.250000;
vert[31].x = -0.978231;vert[31].y = 0.247924;vert[31].z = -0.250000;
vert[32].x = -0.993707;vert[32].y = 0.175446;vert[32].z = -0.250000;
vert[33].x = -0.860080;vert[33].y = 0.208837;vert[33].z = -0.250000;
vert[34].x = -0.871980;vert[34].y = 0.151264;vert[34].z = -0.250000;
vert[35].x = -1.009163;vert[35].y = 0.000000;vert[35].z = -0.250000;
vert[36].x = -0.884057;vert[36].y = 0.041861;vert[36].z = -0.250000;
vert[37].x = -1.008247;vert[37].y = -0.042233;vert[37].z = -0.250000;
vert[38].x = -0.993707;vert[38].y = -0.175446;vert[38].z = -0.250000;
vert[39].x = -0.873978;vert[39].y = -0.139193;vert[39].z = -0.250000;
vert[40].x = -0.860080;vert[40].y = -0.208837;vert[40].z = -0.250000;
vert[41].x = -0.981151;vert[41].y = -0.236062;vert[41].z = -0.250000;
vert[42].x = -0.913189;vert[42].y = -0.429332;vert[42].z = -0.250000;
vert[43].x = -0.904910;vert[43].y = -0.446569;vert[43].z = -0.250000;
vert[44].x = -0.776722;vert[44].y = -0.424326;vert[44].z = -0.250000;
vert[45].x = -0.744685;vert[45].y = -0.478281;vert[45].z = -0.250000;
vert[46].x = -0.839195;vert[46].y = -0.560390;vert[46].z = -0.250000;
vert[47].x = -0.758495;vert[47].y = -0.665560;vert[47].z = -0.250000;
vert[48].x = -0.668697;vert[48].y = -0.579746;vert[48].z = -0.250000;
vert[49].x = -0.648794;vert[49].y = -0.772853;vert[49].z = -0.250000;
vert[50].x = -0.579746;vert[50].y = -0.668697;vert[50].z = -0.250000;
vert[51].x = -0.478281;vert[51].y = -0.744685;vert[51].z = -0.250000;
vert[52].x = -0.504582;vert[52].y = -0.873961;vert[52].z = -0.250000;
vert[53].x = -0.405775;vert[53].y = -0.786546;vert[53].z = -0.250000;
vert[54].x = -0.341684;vert[54].y = -0.816364;vert[54].z = -0.250000;
vert[55].x = -0.430354;vert[55].y = -0.912710;vert[55].z = -0.250000;
vert[56].x = -0.249182;vert[56].y = -0.849270;vert[56].z = -0.250000;
vert[57].x = -0.261191;vert[57].y = -0.974777;vert[57].z = -0.250000;
vert[58].x = -0.000000;vert[58].y = -0.885081;vert[58].z = -0.250000;
vert[59].x = -0.000000;vert[59].y = -1.009163;vert[59].z = -0.250000;
vert[60].x = 0.261191;vert[60].y = -0.974777;vert[60].z = -0.250000;
vert[61].x = 0.249182;vert[61].y = -0.849270;vert[61].z = -0.250000;
vert[62].x = 0.430354;vert[62].y = -0.912710;vert[62].z = -0.250000;
vert[63].x = 0.426658;vert[63].y = -0.914439;vert[63].z = -0.250000;
vert[64].x = 0.648794;vert[64].y = -0.772853;vert[64].z = -0.250000;
vert[65].x = 0.405775;vert[65].y = -0.786546;vert[65].z = -0.250000;
vert[66].x = 0.341684;vert[66].y = -0.816364;vert[66].z = -0.250000;
vert[67].x = 0.168149;vert[67].y = -0.868926;vert[67].z = -0.250000;
vert[68].x = -0.000000;vert[68].y = -0.885081;vert[68].z = -0.250000;
vert[69].x = -0.000000;vert[69].y = -0.885081;vert[69].z = -0.250000;
vert[70].x = -0.168149;vert[70].y = -0.868926;vert[70].z = -0.250000;
vert[71].x = -0.261191;vert[71].y = -0.974777;vert[71].z = -0.250000;
vert[72].x = -0.176105;vert[72].y = -0.993620;vert[72].z = -0.250000;
vert[73].x = -0.089041;vert[73].y = -1.005082;vert[73].z = -0.250000;
vert[74].x = -0.000000;vert[74].y = -1.009163;vert[74].z = -0.250000;
vert[75].x = -0.000000;vert[75].y = -1.009163;vert[75].z = -0.250000;
vert[76].x = 0.089041;vert[76].y = -1.005082;vert[76].z = -0.250000;
vert[77].x = 0.176105;vert[77].y = -0.993620;vert[77].z = -0.250000;
vert[78].x = 0.261191;vert[78].y = -0.974777;vert[78].z = -0.250000;
vert[79].x = 0.707735;vert[79].y = -0.719388;vert[79].z = -0.250000;
vert[80].x = 0.579746;vert[80].y = -0.668697;vert[80].z = -0.250000;
vert[81].x = 0.530304;vert[81].y = -0.708504;vert[81].z = -0.250000;
vert[82].x = 0.861229;vert[82].y = -0.525994;vert[82].z = -0.250000;
vert[83].x = 0.668697;vert[83].y = -0.579746;vert[83].z = -0.250000;
vert[84].x = 0.839195;vert[84].y = -0.560390;vert[84].z = -0.250000;
vert[85].x = 0.744685;vert[85].y = -0.478281;vert[85].z = -0.250000;
vert[86].x = 0.820036;vert[86].y = -0.332775;vert[86].z = -0.250000;
vert[87].x = 0.952738;vert[87].y = -0.332495;vert[87].z = -0.250000;
vert[88].x = 0.955640;vert[88].y = -0.324096;vert[88].z = -0.250000;
vert[89].x = 0.820036;vert[89].y = -0.332775;vert[89].z = -0.250000;
vert[90].x = 0.860080;vert[90].y = -0.208837;vert[90].z = -0.250000;
vert[91].x = 0.873978;vert[91].y = -0.139193;vert[91].z = -0.250000;
vert[92].x = 1.008247;vert[92].y = -0.042233;vert[92].z = -0.250000;
vert[93].x = 1.009163;vert[93].y = -0.000000;vert[93].z = -0.250000;
vert[94].x = 0.884032;vert[94].y = -0.042358;vert[94].z = -0.250000;
vert[95].x = 0.884057;vert[95].y = 0.041861;vert[95].z = -0.250000;
vert[96].x = 0.884057;vert[96].y = 0.041861;vert[96].z = -0.250000;
vert[97].x = 0.883362;vert[97].y = 0.054327;vert[97].z = -0.250000;
vert[98].x = 0.875970;vert[98].y = 0.126006;vert[98].z = -0.250000;
vert[99].x = 0.997675;vert[99].y = 0.151136;vert[99].z = -0.250000;
vert[100].x = 0.993707;vert[100].y = 0.175446;vert[100].z = -0.250000;
vert[101].x = 0.871980;vert[101].y = 0.151264;vert[101].z = -0.250000;
vert[102].x = 0.860080;vert[102].y = 0.208837;vert[102].z = -0.250000;
vert[103].x = 0.955640;vert[103].y = 0.324096;vert[103].z = -0.250000;
vert[104].x = 0.815093;vert[104].y = 0.344706;vert[104].z = -0.250000;
vert[105].x = 0.904910;vert[105].y = 0.446569;vert[105].z = -0.250000;
vert[106].x = 0.744685;vert[106].y = 0.478281;vert[106].z = -0.250000;
vert[107].x = 0.744685;vert[107].y = 0.478281;vert[107].z = -0.250000;
vert[108].x = 0.873961;vert[108].y = 0.504582;vert[108].z = -0.250000;
vert[109].x = 0.758495;vert[109].y = 0.665560;vert[109].z = -0.250000;
vert[110].x = 0.758495;vert[110].y = 0.665560;vert[110].z = -0.250000;
vert[111].x = 0.616872;vert[111].y = 0.634690;vert[111].z = -0.250000;
vert[112].x = 0.579746;vert[112].y = 0.668697;vert[112].z = -0.250000;
vert[113].x = 0.478281;vert[113].y = 0.744685;vert[113].z = -0.250000;
vert[114].x = 0.271534;vert[114].y = -0.678902;vert[114].z = -0.250000;
vert[115].x = 0.258499;vert[115].y = -0.691761;vert[115].z = -0.250000;
vert[116].x = 0.258499;vert[116].y = -0.691761;vert[116].z = -0.250000;
vert[117].x = 0.271534;vert[117].y = -0.678902;vert[117].z = -0.250000;
vert[118].x = 0.243452;vert[118].y = -0.691824;vert[118].z = -0.250000;
vert[119].x = 0.243452;vert[119].y = -0.691824;vert[119].z = -0.250000;
vert[120].x = 0.258499;vert[120].y = -0.691761;vert[120].z = -0.250000;
vert[121].x = 0.281493;vert[121].y = -0.661714;vert[121].z = -0.250000;
vert[122].x = 0.281493;vert[122].y = -0.661714;vert[122].z = -0.250000;
vert[123].x = 0.289508;vert[123].y = -0.641360;vert[123].z = -0.250000;
vert[124].x = 0.294278;vert[124].y = -0.622599;vert[124].z = -0.250000;
vert[125].x = 0.285582;vert[125].y = -0.575673;vert[125].z = -0.250000;
vert[126].x = 0.229601;vert[126].y = -0.687984;vert[126].z = -0.250000;
vert[127].x = 0.294309;vert[127].y = -0.596003;vert[127].z = -0.250000;
vert[128].x = 0.294740;vert[128].y = -0.619036;vert[128].z = -0.250000;
vert[129].x = 0.294740;vert[129].y = -0.619036;vert[129].z = -0.250000;
vert[130].x = 0.196216;vert[130].y = -0.669763;vert[130].z = -0.250000;
vert[131].x = 0.270073;vert[131].y = -0.556930;vert[131].z = -0.250000;
vert[132].x = 0.188907;vert[132].y = -0.661224;vert[132].z = -0.250000;
vert[133].x = 0.189147;vert[133].y = -0.638543;vert[133].z = -0.250000;
vert[134].x = 0.189147;vert[134].y = -0.638543;vert[134].z = -0.250000;
vert[135].x = 0.186464;vert[135].y = -0.650165;vert[135].z = -0.250000;
vert[136].x = 0.186464;vert[136].y = -0.650165;vert[136].z = -0.250000;
vert[137].x = 0.188907;vert[137].y = -0.661224;vert[137].z = -0.250000;
vert[138].x = 0.196216;vert[138].y = -0.669763;vert[138].z = -0.250000;
vert[139].x = 0.229601;vert[139].y = -0.687984;vert[139].z = -0.250000;
vert[140].x = 0.294309;vert[140].y = -0.596003;vert[140].z = -0.250000;
vert[141].x = 0.285582;vert[141].y = -0.575673;vert[141].z = -0.250000;
vert[142].x = 0.270073;vert[142].y = -0.556930;vert[142].z = -0.250000;
vert[143].x = 0.249485;vert[143].y = -0.540037;vert[143].z = -0.250000;
vert[144].x = 0.213541;vert[144].y = -0.594159;vert[144].z = -0.250000;
vert[145].x = 0.209902;vert[145].y = -0.605759;vert[145].z = -0.250000;
vert[146].x = 0.176900;vert[146].y = -0.550178;vert[146].z = -0.250000;
vert[147].x = 0.215714;vert[147].y = -0.520701;vert[147].z = -0.250000;
vert[148].x = 0.212264;vert[148].y = -0.573278;vert[148].z = -0.250000;
vert[149].x = 0.214568;vert[149].y = -0.583145;vert[149].z = -0.250000;
vert[150].x = 0.176900;vert[150].y = -0.550178;vert[150].z = -0.250000;
vert[151].x = 0.212264;vert[151].y = -0.573278;vert[151].z = -0.250000;
vert[152].x = 0.212264;vert[152].y = -0.573278;vert[152].z = -0.250000;
vert[153].x = 0.214568;vert[153].y = -0.583145;vert[153].z = -0.250000;
vert[154].x = 0.213541;vert[154].y = -0.594159;vert[154].z = -0.250000;
vert[155].x = 0.209902;vert[155].y = -0.605759;vert[155].z = -0.250000;
vert[156].x = 0.196193;vert[156].y = -0.512395;vert[156].z = -0.250000;
vert[157].x = 0.196193;vert[157].y = -0.512395;vert[157].z = -0.250000;
vert[158].x = 0.108903;vert[158].y = -0.479313;vert[158].z = -0.250000;
vert[159].x = 0.018871;vert[159].y = -0.451759;vert[159].z = -0.250000;
vert[160].x = -0.043265;vert[160].y = -0.435681;vert[160].z = -0.250000;
vert[161].x = -0.070358;vert[161].y = -0.429009;vert[161].z = -0.250000;
vert[162].x = -0.108071;vert[162].y = -0.420152;vert[162].z = -0.250000;
vert[163].x = -0.175279;vert[163].y = -0.405520;vert[163].z = -0.250000;
vert[164].x = -0.301721;vert[164].y = -0.431607;vert[164].z = -0.250000;
vert[165].x = -0.301721;vert[165].y = -0.431607;vert[165].z = -0.250000;
vert[166].x = -0.311874;vert[166].y = -0.421712;vert[166].z = -0.250000;
vert[167].x = -0.311874;vert[167].y = -0.421712;vert[167].z = -0.250000;
vert[168].x = -0.278435;vert[168].y = -0.439540;vert[168].z = -0.250000;
vert[169].x = -0.278435;vert[169].y = -0.439540;vert[169].z = -0.250000;
vert[170].x = -0.175279;vert[170].y = -0.405520;vert[170].z = -0.250000;
vert[171].x = -0.175279;vert[171].y = -0.405520;vert[171].z = -0.250000;
vert[172].x = -0.167495;vert[172].y = -0.308253;vert[172].z = -0.250000;
vert[173].x = -0.324653;vert[173].y = -0.357921;vert[173].z = -0.250000;
vert[174].x = -0.324653;vert[174].y = -0.357921;vert[174].z = -0.250000;
vert[175].x = -0.318945;vert[175].y = -0.409099;vert[175].z = -0.250000;
vert[176].x = -0.326938;vert[176].y = -0.381991;vert[176].z = -0.250000;
vert[177].x = -0.324653;vert[177].y = -0.357921;vert[177].z = -0.250000;
vert[178].x = -0.326938;vert[178].y = -0.381991;vert[178].z = -0.250000;
vert[179].x = -0.318945;vert[179].y = -0.409099;vert[179].z = -0.250000;
vert[180].x = 0.179689;vert[180].y = -0.165581;vert[180].z = -0.250000;
vert[181].x = 0.271273;vert[181].y = -0.170427;vert[181].z = -0.250000;
vert[182].x = 0.271693;vert[182].y = -0.212410;vert[182].z = -0.250000;
vert[183].x = 0.271693;vert[183].y = -0.212410;vert[183].z = -0.250000;
vert[184].x = 0.263876;vert[184].y = -0.153098;vert[184].z = -0.250000;
vert[185].x = 0.253269;vert[185].y = -0.139893;vert[185].z = -0.250000;
vert[186].x = 0.046810;vert[186].y = -0.229445;vert[186].z = -0.250000;
vert[187].x = 0.274011;vert[187].y = -0.190336;vert[187].z = -0.250000;
vert[188].x = 0.271693;vert[188].y = -0.212410;vert[188].z = -0.250000;
vert[189].x = 0.274011;vert[189].y = -0.190336;vert[189].z = -0.250000;
vert[190].x = 0.271273;vert[190].y = -0.170427;vert[190].z = -0.250000;
vert[191].x = 0.263876;vert[191].y = -0.153098;vert[191].z = -0.250000;
vert[192].x = 0.253269;vert[192].y = -0.139893;vert[192].z = -0.250000;
vert[193].x = 0.240028;vert[193].y = -0.131242;vert[193].z = -0.250000;
vert[194].x = 0.245962;vert[194].y = -0.134339;vert[194].z = -0.153406;
vert[195].x = 0.240028;vert[195].y = -0.131242;vert[195].z = -0.250000;
vert[196].x = 0.179689;vert[196].y = -0.165581;vert[196].z = -0.250000;
vert[197].x = 0.179689;vert[197].y = -0.165581;vert[197].z = -0.250000;
vert[198].x = 0.117916;vert[198].y = -0.198808;vert[198].z = -0.250000;
vert[199].x = 0.224365;vert[199].y = -0.127322;vert[199].z = -0.250000;
vert[200].x = 0.224365;vert[200].y = -0.127322;vert[200].z = -0.250000;
vert[201].x = -0.268960;vert[201].y = 0.001964;vert[201].z = -0.250000;
vert[202].x = -0.282591;vert[202].y = 0.012337;vert[202].z = -0.250000;
vert[203].x = -0.299169;vert[203].y = -0.033914;vert[203].z = -0.250000;
vert[204].x = -0.299169;vert[204].y = -0.033914;vert[204].z = -0.250000;
vert[205].x = -0.268960;vert[205].y = 0.001964;vert[205].z = -0.250000;
vert[206].x = -0.268960;vert[206].y = 0.001964;vert[206].z = -0.250000;
vert[207].x = -0.282591;vert[207].y = 0.012337;vert[207].z = -0.250000;
vert[208].x = -0.292968;vert[208].y = 0.028138;vert[208].z = -0.250000;
vert[209].x = -0.292968;vert[209].y = 0.028138;vert[209].z = -0.250000;
vert[210].x = -0.300386;vert[210].y = 0.047575;vert[210].z = -0.250000;
vert[211].x = -0.333971;vert[211].y = -0.015545;vert[211].z = -0.250000;
vert[212].x = -0.302061;vert[212].y = 0.054052;vert[212].z = -0.250000;
vert[213].x = -0.376961;vert[213].y = 0.054241;vert[213].z = -0.250000;
vert[214].x = -0.359467;vert[214].y = 0.013765;vert[214].z = -0.250000;
vert[215].x = -0.306721;vert[215].y = 0.120786;vert[215].z = -0.250000;
vert[216].x = -0.313823;vert[216].y = 0.131095;vert[216].z = -0.250000;
vert[217].x = -0.324103;vert[217].y = 0.139575;vert[217].z = -0.250000;
vert[218].x = -0.385137;vert[218].y = 0.099080;vert[218].z = -0.250000;
vert[219].x = -0.385137;vert[219].y = 0.099080;vert[219].z = -0.250000;
vert[220].x = -0.376961;vert[220].y = 0.054241;vert[220].z = -0.250000;
vert[221].x = -0.359467;vert[221].y = 0.013765;vert[221].z = -0.250000;
vert[222].x = -0.353536;vert[222].y = 0.002972;vert[222].z = -0.153406;
vert[223].x = -0.333971;vert[223].y = -0.015545;vert[223].z = -0.250000;
vert[224].x = -0.306721;vert[224].y = 0.120786;vert[224].z = -0.250000;
vert[225].x = -0.306721;vert[225].y = 0.120786;vert[225].z = -0.250000;
vert[226].x = -0.313823;vert[226].y = 0.131095;vert[226].z = -0.250000;
vert[227].x = -0.317538;vert[227].y = 0.134790;vert[227].z = -0.153406;
vert[228].x = -0.324103;vert[228].y = 0.139575;vert[228].z = -0.250000;
vert[229].x = -0.351951;vert[229].y = 0.151973;vert[229].z = -0.250000;
vert[230].x = -0.351951;vert[230].y = 0.151973;vert[230].z = -0.250000;
vert[231].x = -0.384761;vert[231].y = 0.150492;vert[231].z = -0.250000;
vert[232].x = -0.384761;vert[232].y = 0.150492;vert[232].z = -0.250000;
vert[233].x = -0.367851;vert[233].y = 0.153622;vert[233].z = -0.250000;
vert[234].x = -0.367851;vert[234].y = 0.153622;vert[234].z = -0.250000;
vert[235].x = -0.384761;vert[235].y = 0.150492;vert[235].z = -0.250000;
vert[236].x = 0.262140;vert[236].y = 0.044895;vert[236].z = -0.250000;
vert[237].x = 0.218108;vert[237].y = 0.044007;vert[237].z = -0.250000;
vert[238].x = 0.218108;vert[238].y = 0.044007;vert[238].z = -0.250000;
vert[239].x = 0.262140;vert[239].y = 0.044895;vert[239].z = -0.250000;
vert[240].x = 0.173382;vert[240].y = 0.051571;vert[240].z = -0.250000;
vert[241].x = 0.218108;vert[241].y = 0.044007;vert[241].z = -0.250000;
vert[242].x = 0.298108;vert[242].y = 0.054040;vert[242].z = -0.250000;
vert[243].x = 0.298108;vert[243].y = 0.054040;vert[243].z = -0.250000;
vert[244].x = 0.289888;vert[244].y = 0.089910;vert[244].z = -0.250000;
vert[245].x = 0.319165;vert[245].y = 0.106029;vert[245].z = -0.250000;
vert[246].x = 0.227795;vert[246].y = 0.076830;vert[246].z = -0.250000;
vert[247].x = 0.166911;vert[247].y = 0.053626;vert[247].z = -0.250000;
vert[248].x = 0.198049;vert[248].y = 0.078671;vert[248].z = -0.250000;
vert[249].x = 0.150380;vert[249].y = 0.097886;vert[249].z = -0.250000;
vert[250].x = 0.134278;vert[250].y = 0.069151;vert[250].z = -0.250000;
vert[251].x = 0.134278;vert[251].y = 0.069151;vert[251].z = -0.250000;
vert[252].x = 0.331909;vert[252].y = 0.072919;vert[252].z = -0.250000;
vert[253].x = 0.331909;vert[253].y = 0.072919;vert[253].z = -0.250000;
vert[254].x = 0.352787;vert[254].y = 0.150919;vert[254].z = -0.250000;
vert[255].x = 0.355245;vert[255].y = 0.096433;vert[255].z = -0.250000;
vert[256].x = 0.355245;vert[256].y = 0.096433;vert[256].z = -0.250000;
vert[257].x = 0.341479;vert[257].y = 0.128778;vert[257].z = -0.250000;
vert[258].x = 0.381724;vert[258].y = 0.148733;vert[258].z = -0.250000;
vert[259].x = 0.381724;vert[259].y = 0.148733;vert[259].z = -0.250000;
vert[260].x = 0.382146;vert[260].y = 0.150905;vert[260].z = -0.250000;
vert[261].x = 0.382354;vert[261].y = 0.214227;vert[261].z = -0.250000;
vert[262].x = 0.385002;vert[262].y = 0.179905;vert[262].z = -0.250000;
vert[263].x = 0.385002;vert[263].y = 0.179905;vert[263].z = -0.250000;
vert[264].x = 0.371515;vert[264].y = 0.247691;vert[264].z = -0.250000;
vert[265].x = 0.358952;vert[265].y = 0.180331;vert[265].z = -0.250000;
vert[266].x = 0.352787;vert[266].y = 0.150919;vert[266].z = -0.250000;
vert[267].x = 0.341479;vert[267].y = 0.128778;vert[267].z = -0.250000;
vert[268].x = 0.329927;vert[268].y = 0.115271;vert[268].z = -0.153406;
vert[269].x = 0.319165;vert[269].y = 0.106029;vert[269].z = -0.250000;
vert[270].x = 0.289888;vert[270].y = 0.089910;vert[270].z = -0.250000;
vert[271].x = 0.258122;vert[271].y = 0.080398;vert[271].z = -0.250000;
vert[272].x = 0.227795;vert[272].y = 0.076830;vert[272].z = -0.250000;
vert[273].x = 0.227795;vert[273].y = 0.076830;vert[273].z = -0.250000;
vert[274].x = 0.198049;vert[274].y = 0.078671;vert[274].z = -0.250000;
vert[275].x = 0.172369;vert[275].y = 0.086675;vert[275].z = -0.250000;
vert[276].x = 0.150380;vert[276].y = 0.097886;vert[276].z = -0.250000;
vert[277].x = 0.133514;vert[277].y = 0.112086;vert[277].z = -0.250000;
vert[278].x = 0.133514;vert[278].y = 0.112086;vert[278].z = -0.250000;
vert[279].x = 0.103342;vert[279].y = 0.094308;vert[279].z = -0.250000;
vert[280].x = 0.110550;vert[280].y = 0.143912;vert[280].z = -0.250000;
vert[281].x = 0.110550;vert[281].y = 0.143912;vert[281].z = -0.250000;
vert[282].x = 0.108555;vert[282].y = 0.150682;vert[282].z = -0.250000;
vert[283].x = 0.083078;vert[283].y = 0.123961;vert[283].z = -0.250000;
vert[284].x = 0.073950;vert[284].y = 0.150958;vert[284].z = -0.250000;
vert[285].x = 0.106825;vert[285].y = 0.163974;vert[285].z = -0.250000;
vert[286].x = 0.107108;vert[286].y = 0.160042;vert[286].z = -0.153406;
vert[287].x = 0.106825;vert[287].y = 0.163974;vert[287].z = -0.250000;
vert[288].x = 0.073950;vert[288].y = 0.150958;vert[288].z = -0.250000;
vert[289].x = 0.083078;vert[289].y = 0.123961;vert[289].z = -0.250000;
vert[290].x = 0.079135;vert[290].y = 0.247712;vert[290].z = -0.250000;
vert[291].x = 0.091410;vert[291].y = 0.109403;vert[291].z = -0.153406;
vert[292].x = 0.103342;vert[292].y = 0.094308;vert[292].z = -0.250000;
vert[293].x = 0.382354;vert[293].y = 0.214227;vert[293].z = -0.250000;
vert[294].x = 0.371515;vert[294].y = 0.247691;vert[294].z = -0.250000;
vert[295].x = 0.355476;vert[295].y = 0.205956;vert[295].z = -0.250000;
vert[296].x = 0.350261;vert[296].y = 0.286163;vert[296].z = -0.250000;
vert[297].x = 0.325827;vert[297].y = 0.256671;vert[297].z = -0.250000;
vert[298].x = 0.325827;vert[298].y = 0.256671;vert[298].z = -0.250000;
vert[299].x = 0.355476;vert[299].y = 0.205956;vert[299].z = -0.250000;
vert[300].x = 0.358952;vert[300].y = 0.180331;vert[300].z = -0.250000;
vert[301].x = 0.350261;vert[301].y = 0.286163;vert[301].z = -0.250000;
vert[302].x = 0.321139;vert[302].y = 0.320277;vert[302].z = -0.250000;
vert[303].x = 0.321139;vert[303].y = 0.320277;vert[303].z = -0.250000;
vert[304].x = 0.291147;vert[304].y = 0.344679;vert[304].z = -0.250000;
vert[305].x = 0.246584;vert[305].y = 0.321080;vert[305].z = -0.250000;
vert[306].x = 0.282353;vert[306].y = 0.350684;vert[306].z = -0.250000;
vert[307].x = 0.274820;vert[307].y = 0.302082;vert[307].z = -0.250000;
vert[308].x = 0.282353;vert[308].y = 0.350684;vert[308].z = -0.250000;
vert[309].x = 0.212037;vert[309].y = 0.390174;vert[309].z = -0.250000;
vert[310].x = 0.149346;vert[310].y = 0.247953;vert[310].z = -0.250000;
vert[311].x = 0.142047;vert[311].y = 0.335576;vert[311].z = -0.250000;
vert[312].x = 0.174965;vert[312].y = 0.363291;vert[312].z = -0.250000;
vert[313].x = 0.174965;vert[313].y = 0.363291;vert[313].z = -0.250000;
vert[314].x = 0.114916;vert[314].y = 0.306837;vert[314].z = -0.250000;
vert[315].x = 0.093798;vert[315].y = 0.277395;vert[315].z = -0.250000;
vert[316].x = 0.093798;vert[316].y = 0.277395;vert[316].z = -0.250000;
vert[317].x = 0.079135;vert[317].y = 0.247712;vert[317].z = -0.250000;
vert[318].x = 0.072362;vert[318].y = 0.218134;vert[318].z = -0.250000;
vert[319].x = 0.108628;vert[319].y = 0.186604;vert[319].z = -0.250000;
vert[320].x = 0.118952;vert[320].y = 0.209381;vert[320].z = -0.250000;
vert[321].x = 0.149346;vert[321].y = 0.247953;vert[321].z = -0.250000;
vert[322].x = 0.246584;vert[322].y = 0.321080;vert[322].z = -0.250000;
vert[323].x = 0.246584;vert[323].y = 0.321080;vert[323].z = -0.250000;
vert[324].x = 0.326628;vert[324].y = 0.388322;vert[324].z = -0.250000;
vert[325].x = 0.282353;vert[325].y = 0.350684;vert[325].z = -0.250000;
vert[326].x = 0.326628;vert[326].y = 0.388322;vert[326].z = -0.250000;
vert[327].x = 0.359033;vert[327].y = 0.429504;vert[327].z = -0.250000;
vert[328].x = 0.365596;vert[328].y = 0.440958;vert[328].z = -0.250000;
vert[329].x = 0.378779;vert[329].y = 0.473242;vert[329].z = -0.250000;
vert[330].x = 0.378779;vert[330].y = 0.473242;vert[330].z = -0.250000;
vert[331].x = 0.343672;vert[331].y = 0.518478;vert[331].z = -0.250000;
vert[332].x = 0.333513;vert[332].y = 0.496813;vert[332].z = -0.250000;
vert[333].x = 0.317613;vert[333].y = 0.475837;vert[333].z = -0.250000;
vert[334].x = 0.296828;vert[334].y = 0.455923;vert[334].z = -0.250000;
vert[335].x = 0.231999;vert[335].y = 0.403718;vert[335].z = -0.250000;
vert[336].x = 0.212037;vert[336].y = 0.390174;vert[336].z = -0.250000;
vert[337].x = 0.212037;vert[337].y = 0.390174;vert[337].z = -0.250000;
vert[338].x = 0.191208;vert[338].y = 0.402475;vert[338].z = -0.250000;
vert[339].x = 0.134928;vert[339].y = 0.394147;vert[339].z = -0.250000;
vert[340].x = 0.153901;vert[340].y = 0.432858;vert[340].z = -0.250000;
vert[341].x = 0.144705;vert[341].y = 0.440866;vert[341].z = -0.250000;
vert[342].x = 0.097351;vert[342].y = 0.440842;vert[342].z = -0.250000;
vert[343].x = 0.137404;vert[343].y = 0.447796;vert[343].z = -0.250000;
vert[344].x = 0.137404;vert[344].y = 0.447796;vert[344].z = -0.250000;
vert[345].x = 0.103261;vert[345].y = 0.431927;vert[345].z = -0.250000;
vert[346].x = 0.081543;vert[346].y = 0.471881;vert[346].z = -0.250000;
vert[347].x = 0.081543;vert[347].y = 0.471881;vert[347].z = -0.250000;
vert[348].x = 0.110848;vert[348].y = 0.484188;vert[348].z = -0.250000;
vert[349].x = 0.101520;vert[349].y = 0.507725;vert[349].z = -0.250000;
vert[350].x = 0.134928;vert[350].y = 0.394147;vert[350].z = -0.250000;
vert[351].x = 0.174965;vert[351].y = 0.363291;vert[351].z = -0.250000;
vert[352].x = 0.385325;vert[352].y = 0.518031;vert[352].z = -0.250000;
vert[353].x = 0.385325;vert[353].y = 0.518031;vert[353].z = -0.250000;
vert[354].x = 0.379962;vert[354].y = 0.561462;vert[354].z = -0.250000;
vert[355].x = 0.379962;vert[355].y = 0.561462;vert[355].z = -0.250000;
vert[356].x = 0.346481;vert[356].y = 0.527824;vert[356].z = -0.153406;
vert[357].x = 0.347605;vert[357].y = 0.537820;vert[357].z = -0.250000;
vert[358].x = 0.343672;vert[358].y = 0.518478;vert[358].z = -0.250000;
vert[359].x = 0.365958;vert[359].y = 0.598870;vert[359].z = -0.250000;
vert[360].x = 0.347716;vert[360].y = 0.540267;vert[360].z = -0.250000;
vert[361].x = 0.347716;vert[361].y = 0.540267;vert[361].z = -0.250000;
vert[362].x = 0.333513;vert[362].y = 0.496813;vert[362].z = -0.250000;
vert[363].x = 0.317613;vert[363].y = 0.475837;vert[363].z = -0.250000;
vert[364].x = 0.365958;vert[364].y = 0.598870;vert[364].z = -0.250000;
vert[365].x = 0.344725;vert[365].y = 0.561636;vert[365].z = -0.250000;
vert[366].x = 0.342593;vert[366].y = 0.629158;vert[366].z = -0.250000;
vert[367].x = 0.348664;vert[367].y = 0.623105;vert[367].z = -0.153406;
vert[368].x = 0.342593;vert[368].y = 0.629158;vert[368].z = -0.250000;
vert[369].x = 0.335592;vert[369].y = 0.580225;vert[369].z = -0.250000;
vert[370].x = 0.261219;vert[370].y = 0.631786;vert[370].z = -0.250000;
vert[371].x = 0.308878;vert[371].y = 0.651806;vert[371].z = -0.250000;
vert[372].x = 0.284194;vert[372].y = 0.623960;vert[372].z = -0.250000;
vert[373].x = 0.305658;vert[373].y = 0.611874;vert[373].z = -0.250000;
vert[374].x = 0.308878;vert[374].y = 0.651806;vert[374].z = -0.250000;
vert[375].x = 0.261219;vert[375].y = 0.631786;vert[375].z = -0.250000;
vert[376].x = 0.265629;vert[376].y = 0.665543;vert[376].z = -0.250000;
vert[377].x = 0.284194;vert[377].y = 0.623960;vert[377].z = -0.250000;
vert[378].x = 0.305658;vert[378].y = 0.611874;vert[378].z = -0.250000;
vert[379].x = 0.322104;vert[379].y = 0.596714;vert[379].z = -0.250000;
vert[380].x = 0.335592;vert[380].y = 0.580225;vert[380].z = -0.250000;
vert[381].x = 0.344725;vert[381].y = 0.561636;vert[381].z = -0.250000;
vert[382].x = 0.265629;vert[382].y = 0.665543;vert[382].z = -0.250000;
vert[383].x = 0.219733;vert[383].y = 0.668716;vert[383].z = -0.250000;
vert[384].x = 0.219733;vert[384].y = 0.668716;vert[384].z = -0.250000;
vert[385].x = 0.207391;vert[385].y = 0.634634;vert[385].z = -0.250000;
vert[386].x = 0.176230;vert[386].y = 0.662507;vert[386].z = -0.250000;
vert[387].x = 0.176230;vert[387].y = 0.662507;vert[387].z = -0.250000;
vert[388].x = 0.121294;vert[388].y = 0.635685;vert[388].z = -0.250000;
vert[389].x = 0.138944;vert[389].y = 0.648011;vert[389].z = -0.250000;
vert[390].x = 0.138944;vert[390].y = 0.648011;vert[390].z = -0.250000;
vert[391].x = 0.121294;vert[391].y = 0.635685;vert[391].z = -0.250000;
vert[392].x = 0.182388;vert[392].y = 0.629517;vert[392].z = -0.250000;
vert[393].x = 0.110727;vert[393].y = 0.580754;vert[393].z = -0.250000;
vert[394].x = 0.106843;vert[394].y = 0.622246;vert[394].z = -0.250000;
vert[395].x = 0.115490;vert[395].y = 0.630832;vert[395].z = -0.153406;
vert[396].x = 0.106843;vert[396].y = 0.622246;vert[396].z = -0.250000;
vert[397].x = 0.139061;vert[397].y = 0.609416;vert[397].z = -0.250000;
vert[398].x = 0.158752;vert[398].y = 0.620926;vert[398].z = -0.250000;
vert[399].x = 0.209294;vert[399].y = 0.634853;vert[399].z = -0.250000;
vert[400].x = 0.096042;vert[400].y = 0.550205;vert[400].z = -0.250000;
vert[401].x = 0.084056;vert[401].y = 0.590063;vert[401].z = -0.250000;
vert[402].x = 0.084056;vert[402].y = 0.590063;vert[402].z = -0.250000;
vert[403].x = 0.101244;vert[403].y = 0.565560;vert[403].z = -0.250000;
vert[404].x = 0.070750;vert[404].y = 0.538029;vert[404].z = -0.250000;
vert[405].x = 0.072194;vert[405].y = 0.554275;vert[405].z = -0.250000;
vert[406].x = 0.079080;vert[406].y = 0.579310;vert[406].z = -0.153406;
vert[407].x = 0.072194;vert[407].y = 0.554275;vert[407].z = -0.250000;
vert[408].x = 0.095845;vert[408].y = 0.537775;vert[408].z = -0.250000;
vert[409].x = 0.072173;vert[409].y = 0.516896;vert[409].z = -0.250000;
vert[410].x = 0.096275;vert[410].y = 0.535027;vert[410].z = -0.153406;
vert[411].x = 0.095845;vert[411].y = 0.537775;vert[411].z = -0.250000;
vert[412].x = 0.096042;vert[412].y = 0.550205;vert[412].z = -0.250000;
vert[413].x = 0.096275;vert[413].y = 0.535027;vert[413].z = -0.250000;
vert[414].x = 0.099786;vert[414].y = 0.562507;vert[414].z = -0.153406;
vert[415].x = 0.101244;vert[415].y = 0.565560;vert[415].z = -0.250000;
vert[416].x = 0.110727;vert[416].y = 0.580754;vert[416].z = -0.250000;
vert[417].x = 0.139061;vert[417].y = 0.609416;vert[417].z = -0.250000;
vert[418].x = 0.158752;vert[418].y = 0.620926;vert[418].z = -0.250000;
vert[419].x = 0.072173;vert[419].y = 0.516896;vert[419].z = -0.250000;
vert[420].x = 0.123093;vert[420].y = 0.464402;vert[420].z = -0.250000;
vert[421].x = 0.110848;vert[421].y = 0.484188;vert[421].z = -0.250000;
vert[422].x = 0.101520;vert[422].y = 0.507725;vert[422].z = -0.250000;
vert[423].x = 0.096275;vert[423].y = 0.535027;vert[423].z = -0.250000;
vert[424].x = 0.070219;vert[424].y = 0.186612;vert[424].z = -0.250000;
vert[425].x = 0.108628;vert[425].y = 0.186604;vert[425].z = -0.250000;
vert[426].x = 0.118952;vert[426].y = 0.209381;vert[426].z = -0.250000;
vert[427].x = 0.000000;vert[427].y = 1.009163;vert[427].z = 0.250000;
vert[428].x = 0.000000;vert[428].y = 1.009163;vert[428].z = 0.250000;
vert[429].x = 0.000000;vert[429].y = 1.009163;vert[429].z = 0.250000;
vert[430].x = 0.261191;vert[430].y = 0.974777;vert[430].z = 0.250000;
vert[431].x = 0.344913;vert[431].y = 0.948298;vert[431].z = 0.250000;
vert[432].x = 0.249182;vert[432].y = 0.849270;vert[432].z = 0.250000;
vert[433].x = 0.000000;vert[433].y = 0.885081;vert[433].z = 0.250000;
vert[434].x = -0.261191;vert[434].y = 0.974777;vert[434].z = 0.250000;
vert[435].x = -0.249182;vert[435].y = 0.849270;vert[435].z = 0.250000;
vert[436].x = -0.344913;vert[436].y = 0.948298;vert[436].z = 0.250000;
vert[437].x = -0.504582;vert[437].y = 0.873961;vert[437].z = 0.250000;
vert[438].x = 0.504582;vert[438].y = 0.873961;vert[438].z = 0.250000;
vert[439].x = 0.576854;vert[439].y = 0.827936;vert[439].z = 0.250000;
vert[440].x = 0.405775;vert[440].y = 0.786546;vert[440].z = 0.250000;
vert[441].x = 0.311052;vert[441].y = 0.828532;vert[441].z = 0.250000;
vert[442].x = 0.168149;vert[442].y = 0.868926;vert[442].z = 0.250000;
vert[443].x = 0.000000;vert[443].y = 0.885081;vert[443].z = 0.250000;
vert[444].x = 0.000000;vert[444].y = 0.885081;vert[444].z = 0.250000;
vert[445].x = -0.168149;vert[445].y = 0.868926;vert[445].z = 0.250000;
vert[446].x = -0.405775;vert[446].y = 0.786546;vert[446].z = 0.250000;
vert[447].x = -0.576854;vert[447].y = 0.827936;vert[447].z = 0.250000;
vert[448].x = -0.405775;vert[448].y = 0.786546;vert[448].z = 0.250000;
vert[449].x = -0.579746;vert[449].y = 0.668697;vert[449].z = 0.250000;
vert[450].x = -0.695028;vert[450].y = 0.731655;vert[450].z = 0.250000;
vert[451].x = -0.758495;vert[451].y = 0.665560;vert[451].z = 0.250000;
vert[452].x = -0.625847;vert[452].y = 0.625847;vert[452].z = 0.250000;
vert[453].x = -0.839195;vert[453].y = 0.560390;vert[453].z = 0.250000;
vert[454].x = -0.744685;vert[454].y = 0.478281;vert[454].z = 0.250000;
vert[455].x = -0.904910;vert[455].y = 0.446569;vert[455].z = 0.250000;
vert[456].x = -0.744685;vert[456].y = 0.478281;vert[456].z = 0.250000;
vert[457].x = -0.836423;vert[457].y = 0.289235;vert[457].z = 0.250000;
vert[458].x = -0.974777;vert[458].y = 0.261191;vert[458].z = 0.250000;
vert[459].x = -0.860080;vert[459].y = 0.208837;vert[459].z = 0.250000;
vert[460].x = -0.993707;vert[460].y = 0.175446;vert[460].z = 0.250000;
vert[461].x = -0.871980;vert[461].y = 0.151264;vert[461].z = 0.250000;
vert[462].x = -0.884057;vert[462].y = 0.041861;vert[462].z = 0.250000;
vert[463].x = -1.009163;vert[463].y = 0.000000;vert[463].z = 0.250000;
vert[464].x = -1.009163;vert[464].y = 0.000000;vert[464].z = 0.250000;
vert[465].x = -0.883783;vert[465].y = -0.047917;vert[465].z = 0.233355;
vert[466].x = -0.875970;vert[466].y = -0.126006;vert[466].z = 0.250000;
vert[467].x = -0.873978;vert[467].y = -0.139193;vert[467].z = 0.250000;
vert[468].x = -0.860080;vert[468].y = -0.208837;vert[468].z = 0.250000;
vert[469].x = -0.873422;vert[469].y = -0.143190;vert[469].z = 0.233355;
vert[470].x = -0.860080;vert[470].y = -0.208837;vert[470].z = 0.250000;
vert[471].x = -0.993707;vert[471].y = -0.175446;vert[471].z = 0.250000;
vert[472].x = -0.853069;vert[472].y = -0.235882;vert[472].z = 0.250000;
vert[473].x = -0.974777;vert[473].y = -0.261191;vert[473].z = 0.250000;
vert[474].x = -0.932243;vert[474].y = -0.386148;vert[474].z = 0.250000;
vert[475].x = -0.913189;vert[475].y = -0.429332;vert[475].z = 0.250000;
vert[476].x = -0.774037;vert[476].y = -0.429214;vert[476].z = 0.250000;
vert[477].x = -0.744685;vert[477].y = -0.478281;vert[477].z = 0.250000;
vert[478].x = -0.904910;vert[478].y = -0.446569;vert[478].z = 0.250000;
vert[479].x = -0.873961;vert[479].y = -0.504582;vert[479].z = 0.233355;
vert[480].x = -0.904910;vert[480].y = -0.446569;vert[480].z = 0.250000;
vert[481].x = -0.873961;vert[481].y = -0.504582;vert[481].z = 0.250000;
vert[482].x = -0.822040;vert[482].y = -0.585372;vert[482].z = 0.233355;
vert[483].x = -0.861229;vert[483].y = -0.525994;vert[483].z = 0.250000;
vert[484].x = -0.674575;vert[484].y = -0.572990;vert[484].z = 0.233355;
vert[485].x = -0.629049;vert[485].y = -0.622628;vert[485].z = 0.250000;
vert[486].x = -0.625847;vert[486].y = -0.625847;vert[486].z = 0.250000;
vert[487].x = -0.758495;vert[487].y = -0.665560;vert[487].z = 0.250000;
vert[488].x = -0.579746;vert[488].y = -0.668697;vert[488].z = 0.250000;
vert[489].x = -0.478281;vert[489].y = -0.744685;vert[489].z = 0.250000;
vert[490].x = -0.648794;vert[490].y = -0.772853;vert[490].z = 0.250000;
vert[491].x = -0.405775;vert[491].y = -0.786546;vert[491].z = 0.250000;
vert[492].x = -0.341684;vert[492].y = -0.816364;vert[492].z = 0.250000;
vert[493].x = -0.504582;vert[493].y = -0.873961;vert[493].z = 0.250000;
vert[494].x = -0.249182;vert[494].y = -0.849270;vert[494].z = 0.250000;
vert[495].x = -0.168149;vert[495].y = -0.868926;vert[495].z = 0.250000;
vert[496].x = -0.000000;vert[496].y = -0.885081;vert[496].z = 0.250000;
vert[497].x = -0.000000;vert[497].y = -0.885081;vert[497].z = 0.250000;
vert[498].x = -0.000000;vert[498].y = -0.885081;vert[498].z = 0.250000;
vert[499].x = 0.168149;vert[499].y = -0.868926;vert[499].z = 0.250000;
vert[500].x = 0.249182;vert[500].y = -0.849270;vert[500].z = 0.250000;
vert[501].x = 0.341684;vert[501].y = -0.816364;vert[501].z = 0.250000;
vert[502].x = 0.478281;vert[502].y = -0.744685;vert[502].z = 0.250000;
vert[503].x = 0.578598;vert[503].y = -0.826716;vert[503].z = 0.250000;
vert[504].x = 0.648794;vert[504].y = -0.772853;vert[504].z = 0.250000;
vert[505].x = 0.430354;vert[505].y = -0.912710;vert[505].z = 0.250000;
vert[506].x = 0.261191;vert[506].y = -0.974777;vert[506].z = 0.250000;
vert[507].x = -0.000000;vert[507].y = -1.009163;vert[507].z = 0.250000;
vert[508].x = -0.261191;vert[508].y = -0.974777;vert[508].z = 0.250000;
vert[509].x = -0.430354;vert[509].y = -0.912710;vert[509].z = 0.250000;
vert[510].x = 0.261191;vert[510].y = -0.974777;vert[510].z = 0.250000;
vert[511].x = 0.176105;vert[511].y = -0.993620;vert[511].z = 0.250000;
vert[512].x = 0.089041;vert[512].y = -1.005082;vert[512].z = 0.250000;
vert[513].x = -0.000000;vert[513].y = -1.009163;vert[513].z = 0.250000;
vert[514].x = -0.000000;vert[514].y = -1.009163;vert[514].z = 0.250000;
vert[515].x = -0.089041;vert[515].y = -1.005082;vert[515].z = 0.250000;
vert[516].x = -0.176105;vert[516].y = -0.993620;vert[516].z = 0.250000;
vert[517].x = -0.261191;vert[517].y = -0.974777;vert[517].z = 0.250000;
vert[518].x = 0.707735;vert[518].y = -0.719388;vert[518].z = 0.250000;
vert[519].x = 0.456307;vert[519].y = -0.758388;vert[519].z = 0.233354;
vert[520].x = 0.515175;vert[520].y = -0.719593;vert[520].z = 0.250000;
vert[521].x = 0.530304;vert[521].y = -0.708504;vert[521].z = 0.250000;
vert[522].x = 0.625847;vert[522].y = -0.625847;vert[522].z = 0.250000;
vert[523].x = 0.762675;vert[523].y = -0.660861;vert[523].z = 0.233354;
vert[524].x = 0.713586;vert[524].y = -0.713586;vert[524].z = 0.250000;
vert[525].x = 0.668697;vert[525].y = -0.579746;vert[525].z = 0.250000;
vert[526].x = 0.794006;vert[526].y = -0.622691;vert[526].z = 0.250000;
vert[527].x = 0.708504;vert[527].y = -0.530304;vert[527].z = 0.250000;
vert[528].x = 0.839195;vert[528].y = -0.560390;vert[528].z = 0.250000;
vert[529].x = 0.732572;vert[529].y = -0.496696;vert[529].z = 0.233354;
vert[530].x = 0.774037;vert[530].y = -0.429214;vert[530].z = 0.250000;
vert[531].x = 0.776722;vert[531].y = -0.424326;vert[531].z = 0.250000;
vert[532].x = 0.913189;vert[532].y = -0.429332;vert[532].z = 0.250000;
vert[533].x = 0.804953;vert[533].y = -0.367794;vert[533].z = 0.250000;
vert[534].x = 0.781980;vert[534].y = -0.414580;vert[534].z = 0.233354;
vert[535].x = 0.804953;vert[535].y = -0.367794;vert[535].z = 0.250000;
vert[536].x = 0.820036;vert[536].y = -0.332775;vert[536].z = 0.250000;
vert[537].x = 0.955640;vert[537].y = -0.324096;vert[537].z = 0.250000;
vert[538].x = 0.860080;vert[538].y = -0.208837;vert[538].z = 0.250000;
vert[539].x = 0.993707;vert[539].y = -0.175446;vert[539].z = 0.250000;
vert[540].x = 0.873978;vert[540].y = -0.139193;vert[540].z = 0.250000;
vert[541].x = 1.008020;vert[541].y = -0.048018;vert[541].z = 0.233354;
vert[542].x = 1.005256;vert[542].y = -0.087723;vert[542].z = 0.250000;
vert[543].x = 0.884032;vert[543].y = -0.042358;vert[543].z = 0.250000;
vert[544].x = 0.884057;vert[544].y = 0.041861;vert[544].z = 0.250000;
vert[545].x = 0.884057;vert[545].y = 0.041861;vert[545].z = 0.250000;
vert[546].x = 1.009163;vert[546].y = -0.000000;vert[546].z = 0.250000;
vert[547].x = 0.883362;vert[547].y = 0.054327;vert[547].z = 0.250000;
vert[548].x = 0.875970;vert[548].y = 0.126006;vert[548].z = 0.250000;
vert[549].x = 1.005256;vert[549].y = 0.087723;vert[549].z = 0.250000;
vert[550].x = 0.997675;vert[550].y = 0.151136;vert[550].z = 0.250000;
vert[551].x = 0.871980;vert[551].y = 0.151264;vert[551].z = 0.250000;
vert[552].x = 0.860080;vert[552].y = 0.208837;vert[552].z = 0.250000;
vert[553].x = 0.993707;vert[553].y = 0.175446;vert[553].z = 0.250000;
vert[554].x = 0.980717;vert[554].y = 0.237919;vert[554].z = 0.233354;
vert[555].x = 0.993707;vert[555].y = 0.175446;vert[555].z = 0.250000;
vert[556].x = 0.978231;vert[556].y = 0.247924;vert[556].z = 0.250000;
vert[557].x = 0.953660;vert[557].y = 0.330065;vert[557].z = 0.233354;
vert[558].x = 0.974777;vert[558].y = 0.261191;vert[558].z = 0.250000;
vert[559].x = 0.815093;vert[559].y = 0.344706;vert[559].z = 0.250000;
vert[560].x = 0.744685;vert[560].y = 0.478281;vert[560].z = 0.250000;
vert[561].x = 0.744685;vert[561].y = 0.478281;vert[561].z = 0.250000;
vert[562].x = 0.904910;vert[562].y = 0.446569;vert[562].z = 0.250000;
vert[563].x = 0.904910;vert[563].y = 0.446569;vert[563].z = 0.250000;
vert[564].x = 0.873961;vert[564].y = 0.504582;vert[564].z = 0.250000;
vert[565].x = 0.674576;vert[565].y = 0.572990;vert[565].z = 0.233354;
vert[566].x = 0.822040;vert[566].y = 0.585372;vert[566].z = 0.233354;
vert[567].x = 0.853776;vert[567].y = 0.537978;vert[567].z = 0.250000;
vert[568].x = 0.625847;vert[568].y = 0.625847;vert[568].z = 0.250000;
vert[569].x = 0.616872;vert[569].y = 0.634690;vert[569].z = 0.250000;
vert[570].x = 0.579746;vert[570].y = 0.668697;vert[570].z = 0.250000;
vert[571].x = 0.758495;vert[571].y = 0.665560;vert[571].z = 0.250000;
vert[572].x = 0.758495;vert[572].y = 0.665560;vert[572].z = 0.250000;
vert[573].x = 0.478281;vert[573].y = 0.744685;vert[573].z = 0.250000;
vert[574].x = 0.243452;vert[574].y = -0.691824;vert[574].z = 0.250000;
vert[575].x = 0.258499;vert[575].y = -0.691761;vert[575].z = 0.250000;
vert[576].x = 0.258499;vert[576].y = -0.691761;vert[576].z = 0.250000;
vert[577].x = 0.243452;vert[577].y = -0.691824;vert[577].z = 0.250000;
vert[578].x = 0.271534;vert[578].y = -0.678902;vert[578].z = 0.250000;
vert[579].x = 0.271534;vert[579].y = -0.678902;vert[579].z = 0.250000;
vert[580].x = 0.258499;vert[580].y = -0.691761;vert[580].z = 0.250000;
vert[581].x = 0.229601;vert[581].y = -0.687984;vert[581].z = 0.250000;
vert[582].x = 0.229601;vert[582].y = -0.687984;vert[582].z = 0.250000;
vert[583].x = 0.196216;vert[583].y = -0.669763;vert[583].z = 0.250000;
vert[584].x = 0.196216;vert[584].y = -0.669763;vert[584].z = 0.250000;
vert[585].x = 0.189147;vert[585].y = -0.638543;vert[585].z = 0.250000;
vert[586].x = 0.188907;vert[586].y = -0.661224;vert[586].z = 0.250000;
vert[587].x = 0.188907;vert[587].y = -0.661224;vert[587].z = 0.250000;
vert[588].x = 0.186464;vert[588].y = -0.650165;vert[588].z = 0.250000;
vert[589].x = 0.186464;vert[589].y = -0.650165;vert[589].z = 0.250000;
vert[590].x = 0.189147;vert[590].y = -0.638543;vert[590].z = 0.250000;
vert[591].x = 0.281493;vert[591].y = -0.661714;vert[591].z = 0.250000;
vert[592].x = 0.294278;vert[592].y = -0.622599;vert[592].z = 0.250000;
vert[593].x = 0.289508;vert[593].y = -0.641360;vert[593].z = 0.250000;
vert[594].x = 0.294309;vert[594].y = -0.596003;vert[594].z = 0.250000;
vert[595].x = 0.294740;vert[595].y = -0.619036;vert[595].z = 0.250000;
vert[596].x = 0.294740;vert[596].y = -0.619036;vert[596].z = 0.250000;
vert[597].x = 0.209902;vert[597].y = -0.605759;vert[597].z = 0.250000;
vert[598].x = 0.213541;vert[598].y = -0.594159;vert[598].z = 0.250000;
vert[599].x = 0.214568;vert[599].y = -0.583145;vert[599].z = 0.250000;
vert[600].x = 0.270073;vert[600].y = -0.556930;vert[600].z = 0.250000;
vert[601].x = 0.285583;vert[601].y = -0.575673;vert[601].z = 0.250000;
vert[602].x = 0.281493;vert[602].y = -0.661714;vert[602].z = 0.250000;
vert[603].x = 0.294309;vert[603].y = -0.596003;vert[603].z = 0.250000;
vert[604].x = 0.209902;vert[604].y = -0.605759;vert[604].z = 0.250000;
vert[605].x = 0.213541;vert[605].y = -0.594159;vert[605].z = 0.250000;
vert[606].x = 0.214568;vert[606].y = -0.583145;vert[606].z = 0.250000;
vert[607].x = 0.212264;vert[607].y = -0.573278;vert[607].z = 0.250000;
vert[608].x = 0.212264;vert[608].y = -0.573278;vert[608].z = 0.250000;
vert[609].x = 0.176900;vert[609].y = -0.550178;vert[609].z = 0.250000;
vert[610].x = 0.212264;vert[610].y = -0.573278;vert[610].z = 0.250000;
vert[611].x = 0.176900;vert[611].y = -0.550178;vert[611].z = 0.250000;
vert[612].x = 0.215715;vert[612].y = -0.520701;vert[612].z = 0.250000;
vert[613].x = 0.249485;vert[613].y = -0.540037;vert[613].z = 0.250000;
vert[614].x = 0.270073;vert[614].y = -0.556930;vert[614].z = 0.250000;
vert[615].x = 0.285583;vert[615].y = -0.575673;vert[615].z = 0.250000;
vert[616].x = 0.196193;vert[616].y = -0.512395;vert[616].z = 0.250000;
vert[617].x = -0.278435;vert[617].y = -0.439540;vert[617].z = 0.250000;
vert[618].x = -0.301721;vert[618].y = -0.431607;vert[618].z = 0.250000;
vert[619].x = -0.278435;vert[619].y = -0.439540;vert[619].z = 0.250000;
vert[620].x = -0.301721;vert[620].y = -0.431607;vert[620].z = 0.250000;
vert[621].x = -0.324653;vert[621].y = -0.357921;vert[621].z = 0.250000;
vert[622].x = -0.311874;vert[622].y = -0.421712;vert[622].z = 0.250000;
vert[623].x = -0.311874;vert[623].y = -0.421712;vert[623].z = 0.250000;
vert[624].x = -0.175279;vert[624].y = -0.405520;vert[624].z = 0.250000;
vert[625].x = -0.070358;vert[625].y = -0.429009;vert[625].z = 0.250000;
vert[626].x = -0.043265;vert[626].y = -0.435681;vert[626].z = 0.250000;
vert[627].x = -0.108071;vert[627].y = -0.420152;vert[627].z = 0.250000;
vert[628].x = 0.018871;vert[628].y = -0.451759;vert[628].z = 0.250000;
vert[629].x = 0.108903;vert[629].y = -0.479313;vert[629].z = 0.250000;
vert[630].x = 0.196193;vert[630].y = -0.512395;vert[630].z = 0.250000;
vert[631].x = -0.324653;vert[631].y = -0.357921;vert[631].z = 0.250000;
vert[632].x = -0.318945;vert[632].y = -0.409099;vert[632].z = 0.250000;
vert[633].x = -0.326938;vert[633].y = -0.381991;vert[633].z = 0.250000;
vert[634].x = -0.326938;vert[634].y = -0.381991;vert[634].z = 0.250000;
vert[635].x = -0.324653;vert[635].y = -0.357921;vert[635].z = 0.250000;
vert[636].x = -0.167495;vert[636].y = -0.308253;vert[636].z = 0.250000;
vert[637].x = -0.175279;vert[637].y = -0.405520;vert[637].z = 0.250000;
vert[638].x = -0.175279;vert[638].y = -0.405520;vert[638].z = 0.250000;
vert[639].x = -0.318945;vert[639].y = -0.409099;vert[639].z = 0.250000;
vert[640].x = 0.179689;vert[640].y = -0.165581;vert[640].z = 0.250000;
vert[641].x = 0.179689;vert[641].y = -0.165581;vert[641].z = 0.250000;
vert[642].x = 0.117916;vert[642].y = -0.198808;vert[642].z = 0.250000;
vert[643].x = 0.179689;vert[643].y = -0.165581;vert[643].z = 0.250000;
vert[644].x = 0.252072;vert[644].y = -0.138831;vert[644].z = 0.250000;
vert[645].x = 0.271693;vert[645].y = -0.212410;vert[645].z = 0.250000;
vert[646].x = 0.240028;vert[646].y = -0.131242;vert[646].z = 0.250000;
vert[647].x = 0.245962;vert[647].y = -0.134339;vert[647].z = 0.233354;
vert[648].x = 0.240028;vert[648].y = -0.131242;vert[648].z = 0.250000;
vert[649].x = 0.252072;vert[649].y = -0.138831;vert[649].z = 0.250000;
vert[650].x = 0.263876;vert[650].y = -0.153098;vert[650].z = 0.250000;
vert[651].x = 0.263876;vert[651].y = -0.153098;vert[651].z = 0.250000;
vert[652].x = 0.271273;vert[652].y = -0.170427;vert[652].z = 0.250000;
vert[653].x = 0.271273;vert[653].y = -0.170427;vert[653].z = 0.250000;
vert[654].x = 0.274011;vert[654].y = -0.190336;vert[654].z = 0.250000;
vert[655].x = 0.274011;vert[655].y = -0.190336;vert[655].z = 0.250000;
vert[656].x = 0.271693;vert[656].y = -0.212410;vert[656].z = 0.250000;
vert[657].x = 0.271693;vert[657].y = -0.212410;vert[657].z = 0.250000;
vert[658].x = 0.046810;vert[658].y = -0.229445;vert[658].z = 0.250000;
vert[659].x = -0.376513;vert[659].y = 0.052725;vert[659].z = 0.250000;
vert[660].x = -0.384760;vert[660].y = 0.150492;vert[660].z = 0.250000;
vert[661].x = -0.384760;vert[661].y = 0.150492;vert[661].z = 0.250000;
vert[662].x = -0.385137;vert[662].y = 0.099080;vert[662].z = 0.250000;
vert[663].x = -0.385137;vert[663].y = 0.099080;vert[663].z = 0.250000;
vert[664].x = -0.384760;vert[664].y = 0.150492;vert[664].z = 0.250000;
vert[665].x = -0.367851;vert[665].y = 0.153622;vert[665].z = 0.250000;
vert[666].x = -0.367851;vert[666].y = 0.153622;vert[666].z = 0.250000;
vert[667].x = -0.349380;vert[667].y = 0.151177;vert[667].z = 0.250000;
vert[668].x = -0.351951;vert[668].y = 0.151973;vert[668].z = 0.250000;
vert[669].x = -0.366664;vert[669].y = 0.153996;vert[669].z = 0.233354;
vert[670].x = -0.351951;vert[670].y = 0.151973;vert[670].z = 0.250000;
vert[671].x = -0.324103;vert[671].y = 0.139575;vert[671].z = 0.250000;
vert[672].x = -0.324103;vert[672].y = 0.139575;vert[672].z = 0.250000;
vert[673].x = -0.313823;vert[673].y = 0.131095;vert[673].z = 0.250000;
vert[674].x = -0.317538;vert[674].y = 0.134790;vert[674].z = 0.233354;
vert[675].x = -0.313823;vert[675].y = 0.131095;vert[675].z = 0.250000;
vert[676].x = -0.306721;vert[676].y = 0.120786;vert[676].z = 0.250000;
vert[677].x = -0.306721;vert[677].y = 0.120786;vert[677].z = 0.250000;
vert[678].x = -0.306721;vert[678].y = 0.120786;vert[678].z = 0.250000;
vert[679].x = -0.302061;vert[679].y = 0.054052;vert[679].z = 0.250000;
vert[680].x = -0.333970;vert[680].y = -0.015545;vert[680].z = 0.250000;
vert[681].x = -0.299169;vert[681].y = -0.033914;vert[681].z = 0.250000;
vert[682].x = -0.300386;vert[682].y = 0.047575;vert[682].z = 0.250000;
vert[683].x = -0.376513;vert[683].y = 0.052725;vert[683].z = 0.250000;
vert[684].x = -0.359467;vert[684].y = 0.013765;vert[684].z = 0.250000;
vert[685].x = -0.359467;vert[685].y = 0.013765;vert[685].z = 0.250000;
vert[686].x = -0.292968;vert[686].y = 0.028138;vert[686].z = 0.250000;
vert[687].x = -0.292968;vert[687].y = 0.028138;vert[687].z = 0.250000;
vert[688].x = -0.282591;vert[688].y = 0.012337;vert[688].z = 0.250000;
vert[689].x = -0.282591;vert[689].y = 0.012337;vert[689].z = 0.250000;
vert[690].x = -0.268959;vert[690].y = 0.001964;vert[690].z = 0.250000;
vert[691].x = -0.268959;vert[691].y = 0.001964;vert[691].z = 0.250000;
vert[692].x = -0.268959;vert[692].y = 0.001964;vert[692].z = 0.250000;
vert[693].x = -0.299169;vert[693].y = -0.033914;vert[693].z = 0.250000;
vert[694].x = -0.312789;vert[694].y = -0.027652;vert[694].z = 0.233354;
vert[695].x = -0.333970;vert[695].y = -0.015545;vert[695].z = 0.250000;
vert[696].x = -0.353536;vert[696].y = 0.002972;vert[696].z = 0.233354;
vert[697].x = 0.224365;vert[697].y = -0.127322;vert[697].z = 0.250000;
vert[698].x = 0.224365;vert[698].y = -0.127322;vert[698].z = 0.250000;
vert[699].x = 0.173382;vert[699].y = 0.051571;vert[699].z = 0.250000;
vert[700].x = 0.218108;vert[700].y = 0.044007;vert[700].z = 0.250000;
vert[701].x = 0.218108;vert[701].y = 0.044007;vert[701].z = 0.250000;
vert[702].x = 0.262140;vert[702].y = 0.044895;vert[702].z = 0.250000;
vert[703].x = 0.262140;vert[703].y = 0.044895;vert[703].z = 0.250000;
vert[704].x = 0.218108;vert[704].y = 0.044007;vert[704].z = 0.250000;
vert[705].x = 0.166911;vert[705].y = 0.053626;vert[705].z = 0.250000;
vert[706].x = 0.150381;vert[706].y = 0.097886;vert[706].z = 0.250000;
vert[707].x = 0.134278;vert[707].y = 0.069151;vert[707].z = 0.250000;
vert[708].x = 0.134278;vert[708].y = 0.069151;vert[708].z = 0.250000;
vert[709].x = 0.298108;vert[709].y = 0.054040;vert[709].z = 0.250000;
vert[710].x = 0.198049;vert[710].y = 0.078671;vert[710].z = 0.250000;
vert[711].x = 0.227795;vert[711].y = 0.076830;vert[711].z = 0.250000;
vert[712].x = 0.298108;vert[712].y = 0.054040;vert[712].z = 0.250000;
vert[713].x = 0.289888;vert[713].y = 0.089910;vert[713].z = 0.250000;
vert[714].x = 0.319165;vert[714].y = 0.106029;vert[714].z = 0.250000;
vert[715].x = 0.331909;vert[715].y = 0.072919;vert[715].z = 0.250000;
vert[716].x = 0.331909;vert[716].y = 0.072919;vert[716].z = 0.250000;
vert[717].x = 0.110550;vert[717].y = 0.143912;vert[717].z = 0.250000;
vert[718].x = 0.103342;vert[718].y = 0.094308;vert[718].z = 0.250000;
vert[719].x = 0.103342;vert[719].y = 0.094308;vert[719].z = 0.250000;
vert[720].x = 0.133514;vert[720].y = 0.112086;vert[720].z = 0.250000;
vert[721].x = 0.073950;vert[721].y = 0.150958;vert[721].z = 0.250000;
vert[722].x = 0.083078;vert[722].y = 0.123961;vert[722].z = 0.250000;
vert[723].x = 0.091410;vert[723].y = 0.109403;vert[723].z = 0.233354;
vert[724].x = 0.083078;vert[724].y = 0.123961;vert[724].z = 0.250000;
vert[725].x = 0.073950;vert[725].y = 0.150958;vert[725].z = 0.250000;
vert[726].x = 0.079136;vert[726].y = 0.247712;vert[726].z = 0.250000;
vert[727].x = 0.073240;vert[727].y = 0.154591;vert[727].z = 0.250000;
vert[728].x = 0.107108;vert[728].y = 0.160042;vert[728].z = 0.233354;
vert[729].x = 0.108556;vert[729].y = 0.150682;vert[729].z = 0.250000;
vert[730].x = 0.110550;vert[730].y = 0.143912;vert[730].z = 0.250000;
vert[731].x = 0.106825;vert[731].y = 0.163974;vert[731].z = 0.250000;
vert[732].x = 0.106825;vert[732].y = 0.163974;vert[732].z = 0.250000;
vert[733].x = 0.133514;vert[733].y = 0.112086;vert[733].z = 0.250000;
vert[734].x = 0.150381;vert[734].y = 0.097886;vert[734].z = 0.250000;
vert[735].x = 0.172369;vert[735].y = 0.086675;vert[735].z = 0.250000;
vert[736].x = 0.198049;vert[736].y = 0.078671;vert[736].z = 0.250000;
vert[737].x = 0.227795;vert[737].y = 0.076830;vert[737].z = 0.250000;
vert[738].x = 0.227795;vert[738].y = 0.076830;vert[738].z = 0.250000;
vert[739].x = 0.258122;vert[739].y = 0.080398;vert[739].z = 0.250000;
vert[740].x = 0.289888;vert[740].y = 0.089910;vert[740].z = 0.250000;
vert[741].x = 0.319165;vert[741].y = 0.106029;vert[741].z = 0.250000;
vert[742].x = 0.341479;vert[742].y = 0.128778;vert[742].z = 0.250000;
vert[743].x = 0.329927;vert[743].y = 0.115271;vert[743].z = 0.233354;
vert[744].x = 0.341479;vert[744].y = 0.128778;vert[744].z = 0.250000;
vert[745].x = 0.355245;vert[745].y = 0.096433;vert[745].z = 0.250000;
vert[746].x = 0.352787;vert[746].y = 0.150919;vert[746].z = 0.250000;
vert[747].x = 0.352787;vert[747].y = 0.150919;vert[747].z = 0.250000;
vert[748].x = 0.381724;vert[748].y = 0.148733;vert[748].z = 0.250000;
vert[749].x = 0.381724;vert[749].y = 0.148733;vert[749].z = 0.250000;
vert[750].x = 0.382354;vert[750].y = 0.214227;vert[750].z = 0.250000;
vert[751].x = 0.385002;vert[751].y = 0.179905;vert[751].z = 0.250000;
vert[752].x = 0.385002;vert[752].y = 0.179905;vert[752].z = 0.250000;
vert[753].x = 0.358952;vert[753].y = 0.180331;vert[753].z = 0.250000;
vert[754].x = 0.371516;vert[754].y = 0.247691;vert[754].z = 0.250000;
vert[755].x = 0.365147;vert[755].y = 0.109373;vert[755].z = 0.233354;
vert[756].x = 0.355245;vert[756].y = 0.096433;vert[756].z = 0.250000;
vert[757].x = 0.382354;vert[757].y = 0.214227;vert[757].z = 0.250000;
vert[758].x = 0.358952;vert[758].y = 0.180331;vert[758].z = 0.250000;
vert[759].x = 0.355476;vert[759].y = 0.205956;vert[759].z = 0.250000;
vert[760].x = 0.355476;vert[760].y = 0.205956;vert[760].z = 0.250000;
vert[761].x = 0.350261;vert[761].y = 0.286163;vert[761].z = 0.250000;
vert[762].x = 0.321139;vert[762].y = 0.320277;vert[762].z = 0.250000;
vert[763].x = 0.325827;vert[763].y = 0.256671;vert[763].z = 0.250000;
vert[764].x = 0.325827;vert[764].y = 0.256671;vert[764].z = 0.250000;
vert[765].x = 0.371516;vert[765].y = 0.247691;vert[765].z = 0.250000;
vert[766].x = 0.350261;vert[766].y = 0.286163;vert[766].z = 0.250000;
vert[767].x = 0.246584;vert[767].y = 0.321080;vert[767].z = 0.250000;
vert[768].x = 0.246584;vert[768].y = 0.321080;vert[768].z = 0.250000;
vert[769].x = 0.246584;vert[769].y = 0.321080;vert[769].z = 0.250000;
vert[770].x = 0.149347;vert[770].y = 0.247953;vert[770].z = 0.250000;
vert[771].x = 0.149347;vert[771].y = 0.247953;vert[771].z = 0.250000;
vert[772].x = 0.093798;vert[772].y = 0.277395;vert[772].z = 0.250000;
vert[773].x = 0.134827;vert[773].y = 0.232150;vert[773].z = 0.250000;
vert[774].x = 0.114917;vert[774].y = 0.306837;vert[774].z = 0.250000;
vert[775].x = 0.079136;vert[775].y = 0.247712;vert[775].z = 0.250000;
vert[776].x = 0.093798;vert[776].y = 0.277395;vert[776].z = 0.250000;
vert[777].x = 0.072362;vert[777].y = 0.218134;vert[777].z = 0.250000;
vert[778].x = 0.118953;vert[778].y = 0.209381;vert[778].z = 0.250000;
vert[779].x = 0.108628;vert[779].y = 0.186604;vert[779].z = 0.250000;
vert[780].x = 0.114917;vert[780].y = 0.306837;vert[780].z = 0.250000;
vert[781].x = 0.174965;vert[781].y = 0.363291;vert[781].z = 0.250000;
vert[782].x = 0.174965;vert[782].y = 0.363291;vert[782].z = 0.250000;
vert[783].x = 0.282353;vert[783].y = 0.350684;vert[783].z = 0.250000;
vert[784].x = 0.212037;vert[784].y = 0.390174;vert[784].z = 0.250000;
vert[785].x = 0.321139;vert[785].y = 0.320277;vert[785].z = 0.250000;
vert[786].x = 0.282353;vert[786].y = 0.350684;vert[786].z = 0.250000;
vert[787].x = 0.326628;vert[787].y = 0.388322;vert[787].z = 0.250000;
vert[788].x = 0.326628;vert[788].y = 0.388322;vert[788].z = 0.250000;
vert[789].x = 0.282353;vert[789].y = 0.350684;vert[789].z = 0.250000;
vert[790].x = 0.134929;vert[790].y = 0.394147;vert[790].z = 0.250000;
vert[791].x = 0.174965;vert[791].y = 0.363291;vert[791].z = 0.250000;
vert[792].x = 0.134929;vert[792].y = 0.394147;vert[792].z = 0.250000;
vert[793].x = 0.097351;vert[793].y = 0.440842;vert[793].z = 0.250000;
vert[794].x = 0.103261;vert[794].y = 0.431927;vert[794].z = 0.250000;
vert[795].x = 0.137404;vert[795].y = 0.447796;vert[795].z = 0.250000;
vert[796].x = 0.101520;vert[796].y = 0.507725;vert[796].z = 0.250000;
vert[797].x = 0.081543;vert[797].y = 0.471881;vert[797].z = 0.250000;
vert[798].x = 0.110848;vert[798].y = 0.484188;vert[798].z = 0.250000;
vert[799].x = 0.081543;vert[799].y = 0.471881;vert[799].z = 0.250000;
vert[800].x = 0.144705;vert[800].y = 0.440866;vert[800].z = 0.250000;
vert[801].x = 0.153901;vert[801].y = 0.432858;vert[801].z = 0.250000;
vert[802].x = 0.137404;vert[802].y = 0.447796;vert[802].z = 0.250000;
vert[803].x = 0.191208;vert[803].y = 0.402475;vert[803].z = 0.250000;
vert[804].x = 0.212037;vert[804].y = 0.390174;vert[804].z = 0.250000;
vert[805].x = 0.212037;vert[805].y = 0.390174;vert[805].z = 0.250000;
vert[806].x = 0.231999;vert[806].y = 0.403718;vert[806].z = 0.250000;
vert[807].x = 0.359033;vert[807].y = 0.429504;vert[807].z = 0.250000;
vert[808].x = 0.296828;vert[808].y = 0.455923;vert[808].z = 0.250000;
vert[809].x = 0.366932;vert[809].y = 0.443735;vert[809].z = 0.233354;
vert[810].x = 0.365596;vert[810].y = 0.440958;vert[810].z = 0.250000;
vert[811].x = 0.378779;vert[811].y = 0.473242;vert[811].z = 0.250000;
vert[812].x = 0.378779;vert[812].y = 0.473242;vert[812].z = 0.250000;
vert[813].x = 0.317613;vert[813].y = 0.475837;vert[813].z = 0.250000;
vert[814].x = 0.333513;vert[814].y = 0.496813;vert[814].z = 0.250000;
vert[815].x = 0.343673;vert[815].y = 0.518478;vert[815].z = 0.250000;
vert[816].x = 0.070219;vert[816].y = 0.186612;vert[816].z = 0.250000;
vert[817].x = 0.118953;vert[817].y = 0.209381;vert[817].z = 0.250000;
vert[818].x = 0.108628;vert[818].y = 0.186604;vert[818].z = 0.250000;
vert[819].x = 0.070750;vert[819].y = 0.538029;vert[819].z = 0.250000;
vert[820].x = 0.072173;vert[820].y = 0.516896;vert[820].z = 0.233354;
vert[821].x = 0.072173;vert[821].y = 0.516896;vert[821].z = 0.250000;
vert[822].x = 0.070750;vert[822].y = 0.538029;vert[822].z = 0.250000;
vert[823].x = 0.072173;vert[823].y = 0.516896;vert[823].z = 0.250000;
vert[824].x = 0.095845;vert[824].y = 0.537775;vert[824].z = 0.250000;
vert[825].x = 0.072194;vert[825].y = 0.554275;vert[825].z = 0.250000;
vert[826].x = 0.072194;vert[826].y = 0.554275;vert[826].z = 0.250000;
vert[827].x = 0.099787;vert[827].y = 0.562507;vert[827].z = 0.233354;
vert[828].x = 0.095845;vert[828].y = 0.537775;vert[828].z = 0.250000;
vert[829].x = 0.096276;vert[829].y = 0.535027;vert[829].z = 0.250000;
vert[830].x = 0.084056;vert[830].y = 0.590063;vert[830].z = 0.250000;
vert[831].x = 0.096042;vert[831].y = 0.550205;vert[831].z = 0.250000;
vert[832].x = 0.096042;vert[832].y = 0.550205;vert[832].z = 0.250000;
vert[833].x = 0.096276;vert[833].y = 0.535027;vert[833].z = 0.250000;
vert[834].x = 0.101520;vert[834].y = 0.507725;vert[834].z = 0.250000;
vert[835].x = 0.110848;vert[835].y = 0.484188;vert[835].z = 0.250000;
vert[836].x = 0.123093;vert[836].y = 0.464402;vert[836].z = 0.250000;
vert[837].x = 0.385325;vert[837].y = 0.518031;vert[837].z = 0.250000;
vert[838].x = 0.384763;vert[838].y = 0.504279;vert[838].z = 0.233354;
vert[839].x = 0.385325;vert[839].y = 0.518031;vert[839].z = 0.250000;
vert[840].x = 0.317613;vert[840].y = 0.475837;vert[840].z = 0.250000;
vert[841].x = 0.333513;vert[841].y = 0.496813;vert[841].z = 0.250000;
vert[842].x = 0.343673;vert[842].y = 0.518478;vert[842].z = 0.250000;
vert[843].x = 0.346481;vert[843].y = 0.527824;vert[843].z = 0.233354;
vert[844].x = 0.347605;vert[844].y = 0.537820;vert[844].z = 0.250000;
vert[845].x = 0.347716;vert[845].y = 0.540267;vert[845].z = 0.250000;
vert[846].x = 0.347716;vert[846].y = 0.540267;vert[846].z = 0.250000;
vert[847].x = 0.378307;vert[847].y = 0.567353;vert[847].z = 0.233354;
vert[848].x = 0.384301;vert[848].y = 0.538036;vert[848].z = 0.250000;
vert[849].x = 0.365958;vert[849].y = 0.598870;vert[849].z = 0.250000;
vert[850].x = 0.379962;vert[850].y = 0.561462;vert[850].z = 0.250000;
vert[851].x = 0.379962;vert[851].y = 0.561462;vert[851].z = 0.250000;
vert[852].x = 0.365958;vert[852].y = 0.598870;vert[852].z = 0.250000;
vert[853].x = 0.342593;vert[853].y = 0.629158;vert[853].z = 0.250000;
vert[854].x = 0.344725;vert[854].y = 0.561636;vert[854].z = 0.250000;
vert[855].x = 0.344725;vert[855].y = 0.561636;vert[855].z = 0.250000;
vert[856].x = 0.335592;vert[856].y = 0.580225;vert[856].z = 0.250000;
vert[857].x = 0.335592;vert[857].y = 0.580225;vert[857].z = 0.250000;
vert[858].x = 0.335513;vert[858].y = 0.635230;vert[858].z = 0.250000;
vert[859].x = 0.322104;vert[859].y = 0.596714;vert[859].z = 0.250000;
vert[860].x = 0.305658;vert[860].y = 0.611874;vert[860].z = 0.250000;
vert[861].x = 0.305658;vert[861].y = 0.611874;vert[861].z = 0.250000;
vert[862].x = 0.284194;vert[862].y = 0.623960;vert[862].z = 0.250000;
vert[863].x = 0.284194;vert[863].y = 0.623960;vert[863].z = 0.250000;
vert[864].x = 0.261219;vert[864].y = 0.631786;vert[864].z = 0.250000;
vert[865].x = 0.261219;vert[865].y = 0.631786;vert[865].z = 0.250000;
vert[866].x = 0.265629;vert[866].y = 0.665543;vert[866].z = 0.250000;
vert[867].x = 0.219733;vert[867].y = 0.668716;vert[867].z = 0.250000;
vert[868].x = 0.295811;vert[868].y = 0.657546;vert[868].z = 0.233354;
vert[869].x = 0.335513;vert[869].y = 0.635230;vert[869].z = 0.250000;
vert[870].x = 0.342593;vert[870].y = 0.629158;vert[870].z = 0.250000;
vert[871].x = 0.308878;vert[871].y = 0.651806;vert[871].z = 0.250000;
vert[872].x = 0.308878;vert[872].y = 0.651806;vert[872].z = 0.250000;
vert[873].x = 0.348664;vert[873].y = 0.623105;vert[873].z = 0.233354;
vert[874].x = 0.265629;vert[874].y = 0.665543;vert[874].z = 0.250000;
vert[875].x = 0.209294;vert[875].y = 0.634853;vert[875].z = 0.250000;
vert[876].x = 0.209294;vert[876].y = 0.634853;vert[876].z = 0.250000;
vert[877].x = 0.176230;vert[877].y = 0.662507;vert[877].z = 0.250000;
vert[878].x = 0.138944;vert[878].y = 0.648011;vert[878].z = 0.250000;
vert[879].x = 0.115490;vert[879].y = 0.630832;vert[879].z = 0.233354;
vert[880].x = 0.138944;vert[880].y = 0.648011;vert[880].z = 0.250000;
vert[881].x = 0.110727;vert[881].y = 0.580754;vert[881].z = 0.250000;
vert[882].x = 0.106843;vert[882].y = 0.622246;vert[882].z = 0.250000;
vert[883].x = 0.106843;vert[883].y = 0.622246;vert[883].z = 0.250000;
vert[884].x = 0.158752;vert[884].y = 0.620926;vert[884].z = 0.250000;
vert[885].x = 0.139061;vert[885].y = 0.609416;vert[885].z = 0.250000;
vert[886].x = 0.170653;vert[886].y = 0.661234;vert[886].z = 0.233354;
vert[887].x = 0.176230;vert[887].y = 0.662507;vert[887].z = 0.250000;
vert[888].x = 0.219733;vert[888].y = 0.668716;vert[888].z = 0.250000;
vert[889].x = 0.101244;vert[889].y = 0.565560;vert[889].z = 0.250000;
vert[890].x = 0.079080;vert[890].y = 0.579310;vert[890].z = 0.233354;
vert[891].x = 0.084056;vert[891].y = 0.590063;vert[891].z = 0.250000;
vert[892].x = 0.158752;vert[892].y = 0.620926;vert[892].z = 0.250000;
vert[893].x = 0.139061;vert[893].y = 0.609416;vert[893].z = 0.250000;
vert[894].x = 0.110727;vert[894].y = 0.580754;vert[894].z = 0.250000;
vert[895].x = 0.101244;vert[895].y = 0.565560;vert[895].z = 0.250000;
faces[0].x = 380;faces[0].y = 859;faces[0].z = 379;
faces[1].x = 98;faces[1].y = 97;faces[1].z = 548;
faces[2].x = 135;faces[2].y = 133;faces[2].z = 130;
faces[3].x = 212;faces[3].y = 678;faces[3].z = 679;
faces[4].x = 106;faces[4].y = 112;faces[4].z = 109;
faces[5].x = 31;faces[5].y = 464;faces[5].z = 458;
faces[6].x = 541;faces[6].y = 92;faces[6].z = 549;
faces[7].x = 118;faces[7].y = 126;faces[7].z = 114;
faces[8].x = 38;faces[8].y = 35;faces[8].z = 36;
faces[9].x = 150;faces[9].y = 617;faces[9].z = 169;
faces[10].x = 301;faces[10].y = 785;faces[10].z = 766;
faces[11].x = 382;faces[11].y = 384;faces[11].z = 868;
faces[12].x = 549;faces[12].y = 542;faces[12].z = 541;
faces[13].x = 66;faces[13].y = 501;faces[13].z = 519;
faces[14].x = 551;faces[14].y = 104;faces[14].z = 101;
faces[15].x = 26;faces[15].y = 458;faces[15].z = 453;
faces[16].x = 100;faces[16].y = 95;faces[16].z = 102;
faces[17].x = 59;faces[17].y = 57;faces[17].z = 58;
faces[18].x = 216;faces[18].y = 215;faces[18].z = 218;
faces[19].x = 147;faces[19].y = 131;faces[19].z = 148;
faces[20].x = 320;faces[20].y = 290;faces[20].z = 310;
faces[21].x = 45;faces[21].y = 50;faces[21].z = 47;
faces[22].x = 71;faces[22].y = 509;faces[22].z = 55;
faces[23].x = 803;faces[23].y = 337;faces[23].z = 338;
faces[24].x = 8;faces[24].y = 19;faces[24].z = 18;
faces[25].x = 93;faces[25].y = 88;faces[25].z = 90;
faces[26].x = 35;faces[26].y = 32;faces[26].z = 36;
faces[27].x = 54;faces[27].y = 495;faces[27].z = 70;
faces[28].x = 472;faces[28].y = 476;faces[28].z = 44;
faces[29].x = 803;faces[29].y = 804;faces[29].z = 337;
faces[30].x = 563;faces[30].y = 108;faces[30].z = 564;
faces[31].x = 894;faces[31].y = 895;faces[31].z = 827;
faces[32].x = 270;faces[32].y = 739;faces[32].z = 740;
faces[33].x = 154;faces[33].y = 155;faces[33].z = 605;
faces[34].x = 10;faces[34].y = 3;faces[34].z = 12;
faces[35].x = 20;faces[35].y = 572;faces[35].z = 110;
faces[36].x = 102;faces[36].y = 106;faces[36].z = 105;
faces[37].x = 894;faces[37].y = 414;faces[37].z = 416;
faces[38].x = 44;faces[38].y = 39;faces[38].z = 472;
faces[39].x = 818;faces[39].y = 286;faces[39].z = 425;
faces[40].x = 367;faces[40].y = 368;faces[40].z = 374;
faces[41].x = 81;faces[41].y = 66;faces[41].z = 519;
faces[42].x = 133;faces[42].y = 145;faces[42].z = 126;
faces[43].x = 551;faces[43].y = 559;faces[43].z = 104;
faces[44].x = 448;faces[44].y = 13;faces[44].z = 445;
faces[45].x = 134;faces[45].y = 589;faces[45].z = 590;
faces[46].x = 136;faces[46].y = 587;faces[46].z = 589;
faces[47].x = 88;faces[47].y = 86;faces[47].z = 90;
faces[48].x = 110;faces[48].y = 572;faces[48].z = 566;
faces[49].x = 543;faces[49].y = 545;faces[49].z = 96;
faces[50].x = 211;faces[50].y = 214;faces[50].z = 208;
faces[51].x = 208;faces[51].y = 214;faces[51].z = 210;
faces[52].x = 120;faces[52].y = 577;faces[52].z = 119;
faces[53].x = 42;faces[53].y = 46;faces[53].z = 479;
faces[54].x = 605;faces[54].y = 606;faces[54].z = 153;
faces[55].x = 501;faces[55].y = 520;faces[55].z = 519;
faces[56].x = 623;faces[56].y = 165;faces[56].z = 620;
faces[57].x = 457;faces[57].y = 34;faces[57].z = 30;
faces[58].x = 808;faces[58].y = 363;faces[58].z = 334;
faces[59].x = 219;faces[59].y = 664;faces[59].z = 235;
faces[60].x = 21;faces[60].y = 447;faces[60].z = 11;
faces[61].x = 0;faces[61].y = 8;faces[61].z = 7;
faces[62].x = 176;faces[62].y = 173;faces[62].z = 175;
faces[63].x = 193;faces[63].y = 185;faces[63].z = 180;
faces[64].x = 191;faces[64].y = 653;faces[64].z = 190;
faces[65].x = 55;faces[65].y = 490;faces[65].z = 49;
faces[66].x = 194;faces[66].y = 192;faces[66].z = 195;
faces[67].x = 26;faces[67].y = 31;faces[67].z = 458;
faces[68].x = 113;faces[68].y = 441;faces[68].z = 17;
faces[69].x = 209;faces[69].y = 689;faces[69].z = 207;
faces[70].x = 209;faces[70].y = 679;faces[70].z = 687;
faces[71].x = 130;faces[71].y = 132;faces[71].z = 135;
faces[72].x = 222;faces[72].y = 221;faces[72].z = 223;
faces[73].x = 667;faces[73].y = 672;faces[73].z = 674;
faces[74].x = 374;faces[74].y = 868;faces[74].z = 367;
faces[75].x = 227;faces[75].y = 226;faces[75].z = 228;
faces[76].x = 32;faces[76].y = 33;faces[76].z = 36;
faces[77].x = 863;faces[77].y = 865;faces[77].z = 375;
faces[78].x = 669;faces[78].y = 234;faces[78].z = 232;
faces[79].x = 227;faces[79].y = 667;faces[79].z = 674;
faces[80].x = 874;faces[80].y = 872;faces[80].z = 868;
faces[81].x = 260;faces[81].y = 749;faces[81].z = 755;
faces[82].x = 247;faces[82].y = 701;faces[82].z = 705;
faces[83].x = 217;faces[83].y = 216;faces[83].z = 218;
faces[84].x = 274;faces[84].y = 737;faces[84].z = 273;
faces[85].x = 150;faces[85].y = 611;faces[85].z = 617;
faces[86].x = 755;faces[86].y = 259;faces[86].z = 260;
faces[87].x = 49;faces[87].y = 490;faces[87].z = 482;
faces[88].x = 87;faces[88].y = 528;faces[88].z = 84;
faces[89].x = 266;faces[89].y = 267;faces[89].z = 268;
faces[90].x = 758;faces[90].y = 266;faces[90].z = 747;
faces[91].x = 268;faces[91].y = 267;faces[91].z = 269;
faces[92].x = 37;faces[92].y = 473;faces[92].z = 464;
faces[93].x = 143;faces[93].y = 614;faces[93].z = 142;
faces[94].x = 734;faces[94].y = 735;faces[94].z = 275;
faces[95].x = 301;faces[95].y = 303;faces[95].z = 785;
faces[96].x = 190;faces[96].y = 655;faces[96].z = 189;
faces[97].x = 278;faces[97].y = 730;faces[97].z = 733;
faces[98].x = 93;faces[98].y = 90;faces[98].z = 95;
faces[99].x = 286;faces[99].y = 282;faces[99].z = 287;
faces[100].x = 203;faces[100].y = 211;faces[100].z = 202;
faces[101].x = 547;faces[101].y = 548;faces[101].z = 97;
faces[102].x = 291;faces[102].y = 289;faces[102].z = 292;
faces[103].x = 704;faces[103].y = 239;faces[103].z = 703;
faces[104].x = 286;faces[104].y = 287;faces[104].z = 425;
faces[105].x = 45;faces[105].y = 43;faces[105].z = 40;
faces[106].x = 54;faces[106].y = 492;faces[106].z = 495;
faces[107].x = 286;faces[107].y = 728;faces[107].z = 730;
faces[108].x = 111;faces[108].y = 573;faces[108].z = 113;
faces[109].x = 32;faces[109].y = 28;faces[109].z = 33;
faces[110].x = 827;faces[110].y = 414;faces[110].z = 894;
faces[111].x = 127;faces[111].y = 128;faces[111].z = 144;
faces[112].x = 111;faces[112].y = 569;faces[112].z = 573;
faces[113].x = 303;faces[113].y = 786;faces[113].z = 785;
faces[114].x = 474;faces[114].y = 42;faces[114].z = 475;
faces[115].x = 60;faces[115].y = 59;faces[115].z = 58;
faces[116].x = 183;faces[116].y = 637;faces[116].z = 171;
faces[117].x = 476;faces[117].y = 485;faces[117].z = 484;
faces[118].x = 479;faces[118].y = 475;faces[118].z = 42;
faces[119].x = 131;faces[119].y = 125;faces[119].z = 149;
faces[120].x = 826;faces[120].y = 820;faces[120].z = 822;
faces[121].x = 215;faces[121].y = 210;faces[121].z = 213;
faces[122].x = 326;faces[122].y = 328;faces[122].z = 809;
faces[123].x = 37;faces[123].y = 41;faces[123].z = 473;
faces[124].x = 255;faces[124].y = 252;faces[124].z = 245;
faces[125].x = 246;faces[125].y = 244;faces[125].z = 236;
faces[126].x = 290;faces[126].y = 315;faces[126].z = 310;
faces[127].x = 345;faces[127].y = 799;faces[127].z = 347;
faces[128].x = 490;faces[128].y = 483;faces[128].z = 482;
faces[129].x = 642;faces[129].y = 643;faces[129].z = 198;
faces[130].x = 20;faces[130].y = 439;faces[130].z = 572;
faces[131].x = 229;faces[131].y = 218;faces[131].z = 233;
faces[132].x = 356;faces[132].y = 357;faces[132].z = 358;
faces[133].x = 362;faces[133].y = 841;faces[133].z = 356;
faces[134].x = 868;faces[134].y = 870;faces[134].z = 873;
faces[135].x = 367;faces[135].y = 364;faces[135].z = 368;
faces[136].x = 775;faces[136].y = 318;faces[136].z = 777;
faces[137].x = 521;faces[137].y = 81;faces[137].z = 519;
faces[138].x = 293;faces[138].y = 757;faces[138].z = 752;
faces[139].x = 18;faces[139].y = 7;faces[139].z = 8;
faces[140].x = 295;faces[140].y = 297;faces[140].z = 264;
faces[141].x = 843;faces[141].y = 844;faces[141].z = 846;
faces[142].x = 190;faces[142].y = 653;faces[142].z = 655;
faces[143].x = 542;faces[143].y = 87;faces[143].z = 92;
faces[144].x = 181;faces[144].y = 187;faces[144].z = 180;
faces[145].x = 788;faces[145].y = 325;faces[145].z = 326;
faces[146].x = 332;faces[146].y = 329;faces[146].z = 333;
faces[147].x = 386;faces[147].y = 383;faces[147].z = 385;
faces[148].x = 60;faces[148].y = 58;faces[148].z = 61;
faces[149].x = 108;faces[149].y = 110;faces[149].z = 566;
faces[150].x = 395;faces[150].y = 391;faces[150].z = 396;
faces[151].x = 879;faces[151].y = 883;faces[151].z = 880;
faces[152].x = 406;faces[152].y = 402;faces[152].z = 407;
faces[153].x = 820;faces[153].y = 890;faces[153].z = 406;
faces[154].x = 47;faces[154].y = 43;faces[154].z = 45;
faces[155].x = 182;faces[155].y = 163;faces[155].z = 180;
faces[156].x = 410;faces[156].y = 413;faces[156].z = 411;
faces[157].x = 414;faces[157].y = 412;faces[157].z = 415;
faces[158].x = 417;faces[158].y = 894;faces[158].z = 416;
faces[159].x = 108;faces[159].y = 563;faces[159].z = 557;
faces[160].x = 28;faces[160].y = 24;faces[160].z = 27;
faces[161].x = 426;faces[161].y = 771;faces[161].z = 817;
faces[162].x = 380;faces[162].y = 857;faces[162].z = 859;
faces[163].x = 264;faces[163].y = 261;faces[163].z = 295;
faces[164].x = 211;faces[164].y = 208;faces[164].z = 202;
faces[165].x = 240;faces[165].y = 250;faces[165].z = 249;
faces[166].x = 31;faces[166].y = 37;faces[166].z = 464;
faces[167].x = 250;faces[167].y = 279;faces[167].z = 277;
faces[168].x = 136;faces[168].y = 137;faces[168].z = 587;
faces[169].x = 11;faces[169].y = 436;faces[169].z = 4;
faces[170].x = 148;faces[170].y = 131;faces[170].z = 149;
faces[171].x = 387;faces[171].y = 390;faces[171].z = 395;
faces[172].x = 17;faces[172].y = 442;faces[172].z = 16;
faces[173].x = 70;faces[173].y = 497;faces[173].z = 69;
faces[174].x = 67;faces[174].y = 498;faces[174].z = 499;
faces[175].x = 16;faces[175].y = 443;faces[175].z = 15;
faces[176].x = 84;faces[176].y = 528;faces[176].z = 523;
faces[177].x = 44;faces[177].y = 476;faces[177].z = 484;
faces[178].x = 212;faces[178].y = 224;faces[178].z = 678;
faces[179].x = 448;faces[179].y = 23;faces[179].z = 13;
faces[180].x = 65;faces[180].y = 80;faces[180].z = 63;
faces[181].x = 262;faces[181].y = 265;faces[181].z = 295;
faces[182].x = 80;faces[182].y = 85;faces[182].z = 79;
faces[183].x = 587;faces[183].y = 138;faces[183].z = 584;
faces[184].x = 448;faces[184].y = 25;faces[184].z = 23;
faces[185].x = 120;faces[185].y = 576;faces[185].z = 577;
faces[186].x = 367;faces[186].y = 868;faces[186].z = 873;
faces[187].x = 689;faces[187].y = 206;faces[187].z = 207;
faces[188].x = 384;faces[188].y = 888;faces[188].z = 868;
faces[189].x = 242;faces[189].y = 236;faces[189].z = 244;
faces[190].x = 452;faces[190].y = 29;faces[190].z = 25;
faces[191].x = 240;faces[191].y = 249;faces[191].z = 248;
faces[192].x = 85;faces[192].y = 86;faces[192].z = 82;
faces[193].x = 50;faces[193].y = 53;faces[193].z = 52;
faces[194].x = 755;faces[194].y = 756;faces[194].z = 716;
faces[195].x = 275;faces[195].y = 276;faces[195].z = 734;
faces[196].x = 399;faces[196].y = 892;faces[196].z = 418;
faces[197].x = 481;faces[197].y = 480;faces[197].z = 479;
faces[198].x = 220;faces[198].y = 663;faces[198].z = 219;
faces[199].x = 634;faces[199].y = 177;faces[199].z = 178;
faces[200].x = 48;faces[200].y = 484;faces[200].z = 486;
faces[201].x = 107;faces[201].y = 561;faces[201].z = 565;
faces[202].x = 315;faces[202].y = 311;faces[202].z = 310;
faces[203].x = 482;faces[203].y = 481;faces[203].z = 479;
faces[204].x = 283;faces[204].y = 284;faces[204].z = 280;
faces[205].x = 155;faces[205].y = 604;faces[205].z = 605;
faces[206].x = 347;faces[206].y = 823;faces[206].z = 419;
faces[207].x = 729;faces[207].y = 730;faces[207].z = 728;
faces[208].x = 201;faces[208].y = 193;faces[208].z = 180;
faces[209].x = 465;faces[209].y = 466;faces[209].z = 467;
faces[210].x = 554;faces[210].y = 550;faces[210].z = 99;
faces[211].x = 469;faces[211].y = 470;faces[211].z = 472;
faces[212].x = 19;faces[212].y = 109;faces[212].z = 112;
faces[213].x = 163;faces[213].y = 147;faces[213].z = 146;
faces[214].x = 100;faces[214].y = 93;faces[214].z = 95;
faces[215].x = 288;faces[215].y = 289;faces[215].z = 291;
faces[216].x = 479;faces[216].y = 480;faces[216].z = 475;
faces[217].x = 274;faces[217].y = 736;faces[217].z = 737;
faces[218].x = 268;faces[218].y = 743;faces[218].z = 747;
faces[219].x = 482;faces[219].y = 483;faces[219].z = 481;
faces[220].x = 572;faces[220].y = 567;faces[220].z = 566;
faces[221].x = 743;faces[221].y = 744;faces[221].z = 747;
faces[222].x = 61;faces[222].y = 65;faces[222].z = 63;
faces[223].x = 484;faces[223].y = 485;faces[223].z = 486;
faces[224].x = 209;faces[224].y = 687;faces[224].z = 689;
faces[225].x = 56;faces[225].y = 57;faces[225].z = 53;
faces[226].x = 112;faces[226].y = 18;faces[226].z = 19;
faces[227].x = 698;faces[227].y = 648;faces[227].z = 647;
faces[228].x = 247;faces[228].y = 241;faces[228].z = 701;
faces[229].x = 51;faces[229].y = 492;faces[229].z = 54;
faces[230].x = 336;faces[230].y = 805;faces[230].z = 806;
faces[231].x = 24;faces[231].y = 22;faces[231].z = 27;
faces[232].x = 67;faces[232].y = 501;faces[232].z = 66;
faces[233].x = 183;faces[233].y = 657;faces[233].z = 637;
faces[234].x = 294;faces[234].y = 766;faces[234].z = 765;
faces[235].x = 41;faces[235].y = 474;faces[235].z = 473;
faces[236].x = 303;faces[236].y = 308;faces[236].z = 786;
faces[237].x = 482;faces[237].y = 46;faces[237].z = 49;
faces[238].x = 755;faces[238].y = 256;faces[238].z = 259;
faces[239].x = 375;faces[239].y = 377;faces[239].z = 863;
faces[240].x = 623;faces[240].y = 167;faces[240].z = 165;
faces[241].x = 219;faces[241].y = 663;faces[241].z = 664;
faces[242].x = 847;faces[242].y = 838;faces[242].z = 353;
faces[243].x = 78;faces[243].y = 505;faces[243].z = 510;
faces[244].x = 236;faces[244].y = 237;faces[244].z = 246;
faces[245].x = 879;faces[245].y = 890;faces[245].z = 883;
faces[246].x = 262;faces[246].y = 258;faces[246].z = 265;
faces[247].x = 296;faces[247].y = 264;faces[247].z = 297;
faces[248].x = 104;faces[248].y = 561;faces[248].z = 107;
faces[249].x = 304;faces[249].y = 302;faces[249].z = 307;
faces[250].x = 261;faces[250].y = 262;faces[250].z = 295;
faces[251].x = 77;faces[251].y = 510;faces[251].z = 511;
faces[252].x = 237;faces[252].y = 240;faces[252].z = 248;
faces[253].x = 424;faces[253].y = 320;faces[253].z = 319;
faces[254].x = 214;faces[254].y = 213;faces[254].z = 210;
faces[255].x = 153;faces[255].y = 608;faces[255].z = 152;
faces[256].x = 519;faces[256].y = 520;faces[256].z = 521;
faces[257].x = 523;faces[257].y = 524;faces[257].z = 518;
faces[258].x = 318;faces[258].y = 727;faces[258].z = 777;
faces[259].x = 232;faces[259].y = 661;faces[259].z = 669;
faces[260].x = 246;faces[260].y = 237;faces[260].z = 248;
faces[261].x = 527;faces[261].y = 532;faces[261].z = 533;
faces[262].x = 276;faces[262].y = 278;faces[262].z = 734;
faces[263].x = 194;faces[263].y = 647;faces[263].z = 651;
faces[264].x = 21;faces[264].y = 453;faces[264].z = 450;
faces[265].x = 191;faces[265].y = 651;faces[265].z = 653;
faces[266].x = 21;faces[266].y = 450;faces[266].z = 447;
faces[267].x = 529;faces[267].y = 530;faces[267].z = 531;
faces[268].x = 34;faces[268].y = 461;faces[268].z = 465;
faces[269].x = 153;faces[269].y = 154;faces[269].z = 605;
faces[270].x = 534;faces[270].y = 535;faces[270].z = 536;
faces[271].x = 143;faces[271].y = 613;faces[271].z = 614;
faces[272].x = 38;faces[272].y = 36;faces[272].z = 40;
faces[273].x = 114;faces[273].y = 115;faces[273].z = 118;
faces[274].x = 39;faces[274].y = 469;faces[274].z = 472;
faces[275].x = 536;faces[275].y = 91;faces[275].z = 89;
faces[276].x = 284;faces[276].y = 285;faces[276].z = 280;
faces[277].x = 363;faces[277].y = 841;faces[277].z = 362;
faces[278].x = 888;faces[278].y = 874;faces[278].z = 868;
faces[279].x = 184;faces[279].y = 181;faces[279].z = 180;
faces[280].x = 344;faces[280].y = 836;faces[280].z = 802;
faces[281].x = 48;faces[281].y = 44;faces[281].z = 484;
faces[282].x = 155;faces[282].y = 590;faces[282].z = 604;
faces[283].x = 566;faces[283].y = 564;faces[283].z = 108;
faces[284].x = 818;faces[284].y = 732;faces[284].z = 728;
faces[285].x = 455;faces[285].y = 459;faces[285].z = 454;
faces[286].x = 554;faces[286].y = 555;faces[286].z = 550;
faces[287].x = 380;faces[287].y = 855;faces[287].z = 857;
faces[288].x = 557;faces[288].y = 558;faces[288].z = 556;
faces[289].x = 227;faces[289].y = 228;faces[289].z = 230;
faces[290].x = 298;faces[290].y = 768;faces[290].z = 323;
faces[291].x = 146;faces[291].y = 164;faces[291].z = 163;
faces[292].x = 58;faces[292].y = 57;faces[292].z = 56;
faces[293].x = 105;faces[293].y = 100;faces[293].z = 102;
faces[294].x = 566;faces[294].y = 567;faces[294].z = 564;
faces[295].x = 63;faces[295].y = 60;faces[295].z = 61;
faces[296].x = 125;faces[296].y = 144;faces[296].z = 149;
faces[297].x = 565;faces[297].y = 568;faces[297].z = 569;
faces[298].x = 704;faces[298].y = 238;faces[298].z = 239;
faces[299].x = 229;faces[299].y = 217;faces[299].z = 218;
faces[300].x = 258;faces[300].y = 255;faces[300].z = 257;
faces[301].x = 279;faces[301].y = 283;faces[301].z = 280;
faces[302].x = 279;faces[302].y = 280;faces[302].z = 277;
faces[303].x = 88;faces[303].y = 82;faces[303].z = 86;
faces[304].x = 525;faces[304].y = 81;faces[304].z = 521;
faces[305].x = 128;faces[305].y = 121;faces[305].z = 145;
faces[306].x = 257;faces[306].y = 255;faces[306].z = 245;
faces[307].x = 33;faces[307].y = 28;faces[307].z = 27;
faces[308].x = 4;faces[308].y = 429;faces[308].z = 1;
faces[309].x = 156;faces[309].y = 613;faces[309].z = 143;
faces[310].x = 302;faces[310].y = 296;faces[310].z = 297;
faces[311].x = 218;faces[311].y = 231;faces[311].z = 233;
faces[312].x = 71;faces[312].y = 517;faces[312].z = 509;
faces[313].x = 129;faces[313].y = 603;faces[313].z = 596;
faces[314].x = 424;faces[314].y = 290;faces[314].z = 320;
faces[315].x = 399;faces[315].y = 865;faces[315].z = 876;
faces[316].x = 180;faces[316].y = 203;faces[316].z = 201;
faces[317].x = 151;faces[317].y = 611;faces[317].z = 150;
faces[318].x = 140;faces[318].y = 615;faces[318].z = 603;
faces[319].x = 692;faces[319].y = 200;faces[319].z = 205;
faces[320].x = 345;faces[320].y = 792;faces[320].z = 794;
faces[321].x = 3;faces[321].y = 0;faces[321].z = 6;
faces[322].x = 126;faces[322].y = 145;faces[322].z = 121;
faces[323].x = 165;faces[323].y = 619;faces[323].z = 620;
faces[324].x = 740;faces[324].y = 741;faces[324].z = 743;
faces[325].x = 826;faces[325].y = 890;faces[325].z = 820;
faces[326].x = 647;faces[326].y = 648;faces[326].z = 649;
faces[327].x = 114;faces[327].y = 126;faces[327].z = 121;
faces[328].x = 334;faces[328].y = 335;faces[328].z = 808;
faces[329].x = 79;faces[329].y = 63;faces[329].z = 80;
faces[330].x = 808;faces[330].y = 335;faces[330].z = 806;
faces[331].x = 134;faces[331].y = 136;faces[331].z = 589;
faces[332].x = 284;faces[332].y = 424;faces[332].z = 285;
faces[333].x = 669;faces[333].y = 670;faces[333].z = 667;
faces[334].x = 674;faces[334].y = 675;faces[334].z = 677;
faces[335].x = 424;faces[335].y = 319;faces[335].z = 285;
faces[336].x = 674;faces[336].y = 672;faces[336].z = 675;
faces[337].x = 809;faces[337].y = 810;faces[337].z = 788;
faces[338].x = 758;faces[338].y = 300;faces[338].z = 266;
faces[339].x = 220;faces[339].y = 221;faces[339].z = 222;
faces[340].x = 230;faces[340].y = 669;faces[340].z = 667;
faces[341].x = 97;faces[341].y = 96;faces[341].z = 545;
faces[342].x = 690;faces[342].y = 688;faces[342].z = 681;
faces[343].x = 414;faces[343].y = 827;faces[343].z = 410;
faces[344].x = 696;faces[344].y = 695;faces[344].z = 685;
faces[345].x = 694;faces[345].y = 695;faces[345].z = 696;
faces[346].x = 101;faces[346].y = 98;faces[346].z = 551;
faces[347].x = 281;faces[347].y = 282;faces[347].z = 286;
faces[348].x = 669;faces[348].y = 666;faces[348].z = 670;
faces[349].x = 187;faces[349].y = 182;faces[349].z = 180;
faces[350].x = 43;faces[350].y = 38;faces[350].z = 40;
faces[351].x = 414;faces[351].y = 415;faces[351].z = 416;
faces[352].x = 251;faces[352].y = 705;faces[352].z = 708;
faces[353].x = 89;faces[353].y = 83;faces[353].z = 529;
faces[354].x = 712;faces[354].y = 703;faces[354].z = 239;
faces[355].x = 73;faces[355].y = 514;faces[355].z = 515;
faces[356].x = 723;faces[356].y = 719;faces[356].z = 724;
faces[357].x = 708;faces[357].y = 719;faces[357].z = 723;
faces[358].x = 723;faces[358].y = 724;faces[358].z = 725;
faces[359].x = 723;faces[359].y = 725;faces[359].z = 727;
faces[360].x = 164;faces[360].y = 175;faces[360].z = 173;
faces[361].x = 728;faces[361].y = 732;faces[361].z = 729;
faces[362].x = 431;faces[362].y = 20;faces[362].z = 9;
faces[363].x = 163;faces[363].y = 164;faces[363].z = 173;
faces[364].x = 2;faces[364].y = 431;faces[364].z = 9;
faces[365].x = 335;faces[365].y = 336;faces[365].z = 806;
faces[366].x = 345;faces[366].y = 794;faces[366].z = 799;
faces[367].x = 274;faces[367].y = 735;faces[367].z = 736;
faces[368].x = 13;faces[368].y = 444;faces[368].z = 445;
faces[369].x = 272;faces[369].y = 739;faces[369].z = 271;
faces[370].x = 6;faces[370].y = 5;faces[370].z = 3;
faces[371].x = 743;faces[371].y = 741;faces[371].z = 744;
faces[372].x = 254;faces[372].y = 265;faces[372].z = 258;
faces[373].x = 641;faces[373].y = 693;faces[373].z = 694;
faces[374].x = 130;faces[374].y = 133;faces[374].z = 126;
faces[375].x = 545;faces[375].y = 547;faces[375].z = 97;
faces[376].x = 580;faces[376].y = 117;faces[376].z = 579;
faces[377].x = 128;faces[377].y = 145;faces[377].z = 144;
faces[378].x = 755;faces[378].y = 749;faces[378].z = 756;
faces[379].x = 52;faces[379].y = 47;faces[379].z = 50;
faces[380].x = 201;faces[380].y = 203;faces[380].z = 202;
faces[381].x = 46;faces[381].y = 482;faces[381].z = 479;
faces[382].x = 730;faces[382].y = 281;faces[382].z = 286;
faces[383].x = 764;faces[383].y = 299;faces[383].z = 760;
faces[384].x = 125;faces[384].y = 127;faces[384].z = 144;
faces[385].x = 758;faces[385].y = 299;faces[385].z = 300;
faces[386].x = 185;faces[386].y = 184;faces[386].z = 180;
faces[387].x = 92;faces[387].y = 99;faces[387].z = 549;
faces[388].x = 173;faces[388].y = 180;faces[388].z = 163;
faces[389].x = 270;faces[389].y = 271;faces[389].z = 739;
faces[390].x = 421;faces[390].y = 836;faces[390].z = 420;
faces[391].x = 788;faces[391].y = 326;faces[391].z = 809;
faces[392].x = 551;faces[392].y = 98;faces[392].z = 548;
faces[393].x = 57;faces[393].y = 52;faces[393].z = 53;
faces[394].x = 209;faces[394].y = 212;faces[394].z = 679;
faces[395].x = 377;faces[395].y = 378;faces[395].z = 863;
faces[396].x = 728;faces[396].y = 286;faces[396].z = 818;
faces[397].x = 142;faces[397].y = 615;faces[397].z = 141;
faces[398].x = 87;faces[398].y = 537;faces[398].z = 528;
faces[399].x = 317;faces[399].y = 776;faces[399].z = 316;
faces[400].x = 244;faces[400].y = 245;faces[400].z = 252;
faces[401].x = 747;faces[401].y = 266;faces[401].z = 268;
faces[402].x = 661;faces[402].y = 666;faces[402].z = 669;
faces[403].x = 771;faces[403].y = 322;faces[403].z = 769;
faces[404].x = 859;faces[404].y = 378;faces[404].z = 379;
faces[405].x = 94;faces[405].y = 540;faces[405].z = 543;
faces[406].x = 119;faces[406].y = 582;faces[406].z = 139;
faces[407].x = 773;faces[407].y = 772;faces[407].z = 726;
faces[408].x = 457;faces[408].y = 461;faces[408].z = 34;
faces[409].x = 351;faces[409].y = 792;faces[409].z = 350;
faces[410].x = 146;faces[410].y = 147;faces[410].z = 148;
faces[411].x = 113;faces[411].y = 573;faces[411].z = 441;
faces[412].x = 278;faces[412].y = 733;faces[412].z = 734;
faces[413].x = 297;faces[413].y = 307;faces[413].z = 302;
faces[414].x = 626;faces[414].y = 628;faces[414].z = 160;
faces[415].x = 809;faces[415].y = 812;faces[415].z = 810;
faces[416].x = 166;faces[416].y = 175;faces[416].z = 164;
faces[417].x = 378;faces[417].y = 861;faces[417].z = 863;
faces[418].x = 109;faces[418].y = 105;faces[418].z = 106;
faces[419].x = 64;faces[419].y = 505;faces[419].z = 62;
faces[420].x = 198;faces[420].y = 186;faces[420].z = 658;
faces[421].x = 330;faces[421].y = 838;faces[421].z = 809;
faces[422].x = 94;faces[422].y = 91;faces[422].z = 540;
faces[423].x = 820;faces[423].y = 821;faces[423].z = 822;
faces[424].x = 278;faces[424].y = 281;faces[424].z = 730;
faces[425].x = 827;faces[425].y = 828;faces[425].z = 829;
faces[426].x = 827;faces[426].y = 832;faces[426].z = 828;
faces[427].x = 138;faces[427].y = 582;faces[427].z = 584;
faces[428].x = 422;faces[428].y = 833;faces[428].z = 834;
faces[429].x = 421;faces[429].y = 834;faces[429].z = 835;
faces[430].x = 3;faces[430].y = 5;faces[430].z = 12;
faces[431].x = 838;faces[431].y = 839;faces[431].z = 812;
faces[432].x = 838;faces[432].y = 812;faces[432].z = 809;
faces[433].x = 843;faces[433].y = 842;faces[433].z = 844;
faces[434].x = 847;faces[434].y = 848;faces[434].z = 839;
faces[435].x = 847;faces[435].y = 851;faces[435].z = 848;
faces[436].x = 213;faces[436].y = 218;faces[436].z = 215;
faces[437].x = 847;faces[437].y = 852;faces[437].z = 851;
faces[438].x = 367;faces[438].y = 873;faces[438].z = 847;
faces[439].x = 230;faces[439].y = 667;faces[439].z = 227;
faces[440].x = 96;faces[440].y = 94;faces[440].z = 543;
faces[441].x = 250;faces[441].y = 277;faces[441].z = 249;
faces[442].x = 10;faces[442].y = 12;faces[442].z = 22;
faces[443].x = 868;faces[443].y = 869;faces[443].z = 870;
faces[444].x = 868;faces[444].y = 872;faces[444].z = 869;
faces[445].x = 873;faces[445].y = 870;faces[445].z = 852;
faces[446].x = 55;faces[446].y = 509;faces[446].z = 490;
faces[447].x = 873;faces[447].y = 852;faces[447].z = 847;
faces[448].x = 639;faces[448].y = 634;faces[448].z = 178;
faces[449].x = 24;faces[449].y = 10;faces[449].z = 22;
faces[450].x = 314;faces[450].y = 776;faces[450].z = 780;
faces[451].x = 82;faces[451].y = 79;faces[451].z = 85;
faces[452].x = 886;faces[452].y = 880;faces[452].z = 887;
faces[453].x = 879;faces[453].y = 880;faces[453].z = 886;
faces[454].x = 384;faces[454].y = 387;faces[454].z = 886;
faces[455].x = 456;faces[455].y = 30;faces[455].z = 29;
faces[456].x = 6;faces[456].y = 0;faces[456].z = 7;
faces[457].x = 417;faces[457].y = 892;faces[457].z = 893;
faces[458].x = 252;faces[458].y = 242;faces[458].z = 244;
faces[459].x = 827;faces[459].y = 895;faces[459].z = 832;
faces[460].x = 890;faces[460].y = 826;faces[460].z = 891;
faces[461].x = 742;faces[461].y = 714;faces[461].z = 745;
faces[462].x = 189;faces[462].y = 656;faces[462].z = 188;
faces[463].x = 602;faces[463].y = 579;faces[463].z = 122;
faces[464].x = 374;faces[464].y = 382;faces[464].z = 868;
faces[465].x = 694;faces[465].y = 222;faces[465].z = 223;
faces[466].x = 808;faces[466].y = 840;faces[466].z = 363;
faces[467].x = 854;faces[467].y = 845;faces[467].z = 853;
faces[468].x = 258;faces[468].y = 257;faces[468].z = 254;
faces[469].x = 818;faces[469].y = 426;faces[469].z = 817;
faces[470].x = 186;faces[470].y = 172;faces[470].z = 636;
faces[471].x = 172;faces[471].y = 174;faces[471].z = 631;
faces[472].x = 636;faces[472].y = 658;faces[472].z = 186;
faces[473].x = 658;faces[473].y = 642;faces[473].z = 198;
faces[474].x = 172;faces[474].y = 631;faces[474].z = 636;
faces[475].x = 643;faces[475].y = 197;faces[475].z = 198;
faces[476].x = 842;faces[476].y = 843;faces[476].z = 841;
faces[477].x = 843;faces[477].y = 356;faces[477].z = 841;
faces[478].x = 356;faces[478].y = 358;faces[478].z = 362;
faces[479].x = 775;faces[479].y = 317;faces[479].z = 318;
faces[480].x = 752;faces[480].y = 749;faces[480].z = 260;
faces[481].x = 260;faces[481].y = 263;faces[481].z = 752;
faces[482].x = 263;faces[482].y = 293;faces[482].z = 752;
faces[483].x = 846;faces[483].y = 855;faces[483].z = 843;
faces[484].x = 855;faces[484].y = 381;faces[484].z = 356;
faces[485].x = 361;faces[485].y = 357;faces[485].z = 356;
faces[486].x = 855;faces[486].y = 356;faces[486].z = 843;
faces[487].x = 381;faces[487].y = 361;faces[487].z = 356;
faces[488].x = 92;faces[488].y = 541;faces[488].z = 542;
faces[489].x = 542;faces[489].y = 537;faces[489].z = 87;
faces[490].x = 788;faces[490].y = 789;faces[490].z = 325;
faces[491].x = 346;faces[491].y = 404;faces[491].z = 349;
faces[492].x = 404;faces[492].y = 405;faces[492].z = 400;
faces[493].x = 405;faces[493].y = 401;faces[493].z = 403;
faces[494].x = 401;faces[494].y = 394;faces[494].z = 393;
faces[495].x = 405;faces[495].y = 403;faces[495].z = 400;
faces[496].x = 394;faces[496].y = 388;faces[496].z = 397;
faces[497].x = 388;faces[497].y = 392;faces[497].z = 398;
faces[498].x = 388;faces[498].y = 398;faces[498].z = 397;
faces[499].x = 404;faces[499].y = 400;faces[499].z = 408;
faces[500].x = 339;faces[500].y = 342;faces[500].z = 343;
faces[501].x = 342;faces[501].y = 346;faces[501].z = 348;
faces[502].x = 343;faces[502].y = 342;faces[502].z = 348;
faces[503].x = 310;faces[503].y = 311;faces[503].z = 305;
faces[504].x = 311;faces[504].y = 312;faces[504].z = 305;
faces[505].x = 312;faces[505].y = 339;faces[505].z = 309;
faces[506].x = 394;faces[506].y = 397;faces[506].z = 393;
faces[507].x = 304;faces[507].y = 307;faces[507].z = 306;
faces[508].x = 307;faces[508].y = 305;faces[508].z = 306;
faces[509].x = 327;faces[509].y = 324;faces[509].z = 333;
faces[510].x = 324;faces[510].y = 306;faces[510].z = 309;
faces[511].x = 333;faces[511].y = 324;faces[511].z = 309;
faces[512].x = 305;faces[512].y = 312;faces[512].z = 309;
faces[513].x = 339;faces[513].y = 343;faces[513].z = 309;
faces[514].x = 393;faces[514].y = 403;faces[514].z = 401;
faces[515].x = 306;faces[515].y = 305;faces[515].z = 309;
faces[516].x = 352;faces[516].y = 329;faces[516].z = 331;
faces[517].x = 329;faces[517].y = 327;faces[517].z = 333;
faces[518].x = 331;faces[518].y = 329;faces[518].z = 332;
faces[519].x = 359;faces[519].y = 354;faces[519].z = 365;
faces[520].x = 354;faces[520].y = 352;faces[520].z = 360;
faces[521].x = 371;faces[521].y = 366;faces[521].z = 373;
faces[522].x = 366;faces[522].y = 359;faces[522].z = 369;
faces[523].x = 373;faces[523].y = 366;faces[523].z = 369;
faces[524].x = 383;faces[524].y = 376;faces[524].z = 370;
faces[525].x = 376;faces[525].y = 371;faces[525].z = 372;
faces[526].x = 370;faces[526].y = 385;faces[526].z = 383;
faces[527].x = 372;faces[527].y = 370;faces[527].z = 376;
faces[528].x = 372;faces[528].y = 371;faces[528].z = 373;
faces[529].x = 354;faces[529].y = 360;faces[529].z = 365;
faces[530].x = 404;faces[530].y = 408;faces[530].z = 349;
faces[531].x = 346;faces[531].y = 349;faces[531].z = 348;
faces[532].x = 365;faces[532].y = 369;faces[532].z = 359;
faces[533].x = 331;faces[533].y = 360;faces[533].z = 352;
faces[534].x = 385;faces[534].y = 392;faces[534].z = 386;
faces[535].x = 392;faces[535].y = 388;faces[535].z = 389;
faces[536].x = 386;faces[536].y = 392;faces[536].z = 389;
faces[537].x = 406;faces[537].y = 407;faces[537].z = 409;
faces[538].x = 409;faces[538].y = 820;faces[538].z = 406;
faces[539].x = 417;faces[539].y = 893;faces[539].z = 894;
faces[540].x = 557;faces[540].y = 103;faces[540].z = 108;
faces[541].x = 563;faces[541].y = 558;faces[541].z = 557;
faces[542].x = 426;faces[542].y = 321;faces[542].z = 771;
faces[543].x = 11;faces[543].y = 447;faces[543].z = 436;
faces[544].x = 390;faces[544].y = 391;faces[544].z = 395;
faces[545].x = 395;faces[545].y = 879;faces[545].z = 886;
faces[546].x = 886;faces[546].y = 387;faces[546].z = 395;
faces[547].x = 17;faces[547].y = 441;faces[547].z = 442;
faces[548].x = 70;faces[548].y = 495;faces[548].z = 497;
faces[549].x = 67;faces[549].y = 68;faces[549].z = 498;
faces[550].x = 16;faces[550].y = 442;faces[550].z = 443;
faces[551].x = 504;faces[551].y = 64;faces[551].z = 518;
faces[552].x = 64;faces[552].y = 84;faces[552].z = 523;
faces[553].x = 528;faces[553].y = 524;faces[553].z = 523;
faces[554].x = 523;faces[554].y = 518;faces[554].z = 64;
faces[555].x = 587;faces[555].y = 137;faces[555].z = 138;
faces[556].x = 448;faces[556].y = 452;faces[556].z = 25;
faces[557].x = 689;faces[557].y = 691;faces[557].z = 206;
faces[558].x = 452;faces[558].y = 456;faces[558].z = 29;
faces[559].x = 716;faces[559].y = 253;faces[559].z = 755;
faces[560].x = 253;faces[560].y = 256;faces[560].z = 755;
faces[561].x = 399;faces[561].y = 876;faces[561].z = 892;
faces[562].x = 220;faces[562].y = 683;faces[562].z = 663;
faces[563].x = 634;faces[563].y = 635;faces[563].z = 177;
faces[564].x = 489;faces[564].y = 51;faces[564].z = 486;
faces[565].x = 51;faces[565].y = 48;faces[565].z = 486;
faces[566].x = 561;faces[566].y = 568;faces[566].z = 565;
faces[567].x = 565;faces[567].y = 569;faces[567].z = 111;
faces[568].x = 111;faces[568].y = 107;faces[568].z = 565;
faces[569].x = 347;faces[569].y = 799;faces[569].z = 823;
faces[570].x = 549;faces[570].y = 99;faces[570].z = 550;
faces[571].x = 99;faces[571].y = 103;faces[571].z = 554;
faces[572].x = 103;faces[572].y = 557;faces[572].z = 554;
faces[573].x = 557;faces[573].y = 556;faces[573].z = 554;
faces[574].x = 556;faces[574].y = 555;faces[574].z = 554;
faces[575].x = 723;faces[575].y = 727;faces[575].z = 291;
faces[576].x = 727;faces[576].y = 288;faces[576].z = 291;
faces[577].x = 647;faces[577].y = 194;faces[577].z = 698;
faces[578].x = 194;faces[578].y = 195;faces[578].z = 199;
faces[579].x = 698;faces[579].y = 194;faces[579].z = 199;
faces[580].x = 51;faces[580].y = 489;faces[580].z = 492;
faces[581].x = 67;faces[581].y = 499;faces[581].z = 501;
faces[582].x = 765;faces[582].y = 757;faces[582].z = 293;
faces[583].x = 294;faces[583].y = 301;faces[583].z = 766;
faces[584].x = 765;faces[584].y = 293;faces[584].z = 294;
faces[585].x = 41;faces[585].y = 42;faces[585].z = 474;
faces[586].x = 353;faces[586].y = 355;faces[586].z = 847;
faces[587].x = 847;faces[587].y = 839;faces[587].z = 838;
faces[588].x = 78;faces[588].y = 62;faces[588].z = 505;
faces[589].x = 879;faces[589].y = 395;faces[589].z = 406;
faces[590].x = 395;faces[590].y = 396;faces[590].z = 406;
faces[591].x = 396;faces[591].y = 402;faces[591].z = 406;
faces[592].x = 406;faces[592].y = 890;faces[592].z = 879;
faces[593].x = 890;faces[593].y = 891;faces[593].z = 883;
faces[594].x = 104;faces[594].y = 559;faces[594].z = 561;
faces[595].x = 75;faces[595].y = 76;faces[595].z = 513;
faces[596].x = 76;faces[596].y = 77;faces[596].z = 512;
faces[597].x = 513;faces[597].y = 76;faces[597].z = 512;
faces[598].x = 77;faces[598].y = 78;faces[598].z = 510;
faces[599].x = 511;faces[599].y = 512;faces[599].z = 77;
faces[600].x = 153;faces[600].y = 606;faces[600].z = 608;
faces[601].x = 318;faces[601].y = 288;faces[601].z = 727;
faces[602].x = 527;faces[602].y = 526;faces[602].z = 532;
faces[603].x = 647;faces[603].y = 649;faces[603].z = 651;
faces[604].x = 651;faces[604].y = 191;faces[604].z = 194;
faces[605].x = 191;faces[605].y = 192;faces[605].z = 194;
faces[606].x = 21;faces[606].y = 26;faces[606].z = 453;
faces[607].x = 461;faces[607].y = 466;faces[607].z = 465;
faces[608].x = 465;faces[608].y = 467;faces[608].z = 469;
faces[609].x = 467;faces[609].y = 470;faces[609].z = 469;
faces[610].x = 469;faces[610].y = 39;faces[610].z = 465;
faces[611].x = 39;faces[611].y = 34;faces[611].z = 465;
faces[612].x = 536;faces[612].y = 540;faces[612].z = 91;
faces[613].x = 363;faces[613].y = 840;faces[613].z = 841;
faces[614].x = 803;faces[614].y = 338;faces[614].z = 340;
faces[615].x = 341;faces[615].y = 344;faces[615].z = 802;
faces[616].x = 344;faces[616].y = 420;faces[616].z = 836;
faces[617].x = 803;faces[617].y = 340;faces[617].z = 801;
faces[618].x = 340;faces[618].y = 341;faces[618].z = 801;
faces[619].x = 802;faces[619].y = 800;faces[619].z = 341;
faces[620].x = 800;faces[620].y = 801;faces[620].z = 341;
faces[621].x = 155;faces[621].y = 134;faces[621].z = 590;
faces[622].x = 538;faces[622].y = 533;faces[622].z = 532;
faces[623].x = 532;faces[623].y = 539;faces[623].z = 538;
faces[624].x = 539;faces[624].y = 546;faces[624].z = 544;
faces[625].x = 538;faces[625].y = 539;faces[625].z = 544;
faces[626].x = 546;faces[626].y = 553;faces[626].z = 544;
faces[627].x = 553;faces[627].y = 562;faces[627].z = 552;
faces[628].x = 544;faces[628].y = 553;faces[628].z = 552;
faces[629].x = 562;faces[629].y = 571;faces[629].z = 560;
faces[630].x = 571;faces[630].y = 438;faces[630].z = 570;
faces[631].x = 560;faces[631].y = 571;faces[631].z = 570;
faces[632].x = 438;faces[632].y = 430;faces[632].z = 440;
faces[633].x = 430;faces[633].y = 427;faces[633].z = 433;
faces[634].x = 432;faces[634].y = 430;faces[634].z = 433;
faces[635].x = 427;faces[635].y = 434;faces[635].z = 433;
faces[636].x = 434;faces[636].y = 437;faces[636].z = 446;
faces[637].x = 433;faces[637].y = 434;faces[637].z = 435;
faces[638].x = 437;faces[638].y = 451;faces[638].z = 449;
faces[639].x = 451;faces[639].y = 455;faces[639].z = 454;
faces[640].x = 449;faces[640].y = 451;faces[640].z = 454;
faces[641].x = 455;faces[641].y = 460;faces[641].z = 459;
faces[642].x = 460;faces[642].y = 463;faces[642].z = 462;
faces[643].x = 463;faces[643].y = 471;faces[643].z = 462;
faces[644].x = 471;faces[644].y = 478;faces[644].z = 468;
faces[645].x = 478;faces[645].y = 487;faces[645].z = 477;
faces[646].x = 487;faces[646].y = 493;faces[646].z = 488;
faces[647].x = 493;faces[647].y = 508;faces[647].z = 491;
faces[648].x = 508;faces[648].y = 507;faces[648].z = 496;
faces[649].x = 507;faces[649].y = 506;faces[649].z = 496;
faces[650].x = 506;faces[650].y = 503;faces[650].z = 502;
faces[651].x = 500;faces[651].y = 506;faces[651].z = 502;
faces[652].x = 503;faces[652].y = 526;faces[652].z = 522;
faces[653].x = 526;faces[653].y = 527;faces[653].z = 522;
faces[654].x = 460;faces[654].y = 462;faces[654].z = 459;
faces[655].x = 471;faces[655].y = 468;faces[655].z = 462;
faces[656].x = 503;faces[656].y = 522;faces[656].z = 502;
faces[657].x = 560;faces[657].y = 552;faces[657].z = 562;
faces[658].x = 506;faces[658].y = 500;faces[658].z = 496;
faces[659].x = 508;faces[659].y = 496;faces[659].z = 494;
faces[660].x = 508;faces[660].y = 494;faces[660].z = 491;
faces[661].x = 440;faces[661].y = 570;faces[661].z = 438;
faces[662].x = 432;faces[662].y = 440;faces[662].z = 430;
faces[663].x = 493;faces[663].y = 491;faces[663].z = 488;
faces[664].x = 487;faces[664].y = 488;faces[664].z = 477;
faces[665].x = 435;faces[665].y = 434;faces[665].z = 446;
faces[666].x = 478;faces[666].y = 477;faces[666].z = 468;
faces[667].x = 449;faces[667].y = 446;faces[667].z = 437;
faces[668].x = 380;faces[668].y = 381;faces[668].z = 855;
faces[669].x = 298;faces[669].y = 764;faces[669].z = 768;
faces[670].x = 525;faces[670].y = 83;faces[670].z = 81;
faces[671].x = 4;faces[671].y = 436;faces[671].z = 429;
faces[672].x = 156;faces[672].y = 616;faces[672].z = 613;
faces[673].x = 129;faces[673].y = 140;faces[673].z = 603;
faces[674].x = 399;faces[674].y = 375;faces[674].z = 865;
faces[675].x = 151;faces[675].y = 610;faces[675].z = 611;
faces[676].x = 140;faces[676].y = 141;faces[676].z = 615;
faces[677].x = 692;faces[677].y = 697;faces[677].z = 200;
faces[678].x = 345;faces[678].y = 350;faces[678].z = 792;
faces[679].x = 165;faces[679].y = 168;faces[679].z = 619;
faces[680].x = 743;faces[680].y = 268;faces[680].z = 740;
faces[681].x = 268;faces[681].y = 269;faces[681].z = 270;
faces[682].x = 740;faces[682].y = 268;faces[682].z = 270;
faces[683].x = 677;faces[683].y = 225;faces[683].z = 227;
faces[684].x = 225;faces[684].y = 226;faces[684].z = 227;
faces[685].x = 227;faces[685].y = 674;faces[685].z = 677;
faces[686].x = 222;faces[686].y = 696;faces[686].z = 683;
faces[687].x = 696;faces[687].y = 685;faces[687].z = 683;
faces[688].x = 683;faces[688].y = 220;faces[688].z = 222;
faces[689].x = 230;faces[689].y = 234;faces[689].z = 669;
faces[690].x = 682;faces[690].y = 676;faces[690].z = 659;
faces[691].x = 676;faces[691].y = 673;faces[691].z = 662;
faces[692].x = 659;faces[692].y = 676;faces[692].z = 662;
faces[693].x = 673;faces[693].y = 671;faces[693].z = 662;
faces[694].x = 671;faces[694].y = 668;faces[694].z = 662;
faces[695].x = 668;faces[695].y = 665;faces[695].z = 662;
faces[696].x = 665;faces[696].y = 660;faces[696].z = 662;
faces[697].x = 659;faces[697].y = 684;faces[697].z = 682;
faces[698].x = 684;faces[698].y = 680;faces[698].z = 686;
faces[699].x = 682;faces[699].y = 684;faces[699].z = 686;
faces[700].x = 680;faces[700].y = 681;faces[700].z = 688;
faces[701].x = 681;faces[701].y = 640;faces[701].z = 690;
faces[702].x = 640;faces[702].y = 621;faces[702].z = 624;
faces[703].x = 621;faces[703].y = 633;faces[703].z = 632;
faces[704].x = 624;faces[704].y = 621;faces[704].z = 618;
faces[705].x = 632;faces[705].y = 622;faces[705].z = 618;
faces[706].x = 618;faces[706].y = 609;faces[706].z = 624;
faces[707].x = 632;faces[707].y = 618;faces[707].z = 621;
faces[708].x = 597;faces[708].y = 585;faces[708].z = 581;
faces[709].x = 585;faces[709].y = 588;faces[709].z = 583;
faces[710].x = 588;faces[710].y = 586;faces[710].z = 583;
faces[711].x = 583;faces[711].y = 581;faces[711].z = 585;
faces[712].x = 581;faces[712].y = 574;faces[712].z = 578;
faces[713].x = 574;faces[713].y = 575;faces[713].z = 578;
faces[714].x = 578;faces[714].y = 591;faces[714].z = 581;
faces[715].x = 591;faces[715].y = 595;faces[715].z = 597;
faces[716].x = 597;faces[716].y = 595;faces[716].z = 598;
faces[717].x = 595;faces[717].y = 594;faces[717].z = 598;
faces[718].x = 594;faces[718].y = 601;faces[718].z = 598;
faces[719].x = 601;faces[719].y = 600;faces[719].z = 599;
faces[720].x = 600;faces[720].y = 612;faces[720].z = 607;
faces[721].x = 624;faces[721].y = 645;faces[721].z = 640;
faces[722].x = 645;faces[722].y = 654;faces[722].z = 640;
faces[723].x = 600;faces[723].y = 607;faces[723].z = 599;
faces[724].x = 612;faces[724].y = 624;faces[724].z = 609;
faces[725].x = 597;faces[725].y = 581;faces[725].z = 591;
faces[726].x = 654;faces[726].y = 652;faces[726].z = 640;
faces[727].x = 599;faces[727].y = 598;faces[727].z = 601;
faces[728].x = 652;faces[728].y = 650;faces[728].z = 640;
faces[729].x = 609;faces[729].y = 607;faces[729].z = 612;
faces[730].x = 650;faces[730].y = 644;faces[730].z = 640;
faces[731].x = 644;faces[731].y = 646;faces[731].z = 640;
faces[732].x = 688;faces[732].y = 686;faces[732].z = 680;
faces[733].x = 640;faces[733].y = 646;faces[733].z = 690;
faces[734].x = 829;faces[734].y = 410;faces[734].z = 827;
faces[735].x = 410;faces[735].y = 411;faces[735].z = 412;
faces[736].x = 414;faces[736].y = 410;faces[736].z = 412;
faces[737].x = 251;faces[737].y = 247;faces[737].z = 705;
faces[738].x = 525;faces[738].y = 530;faces[738].z = 529;
faces[739].x = 529;faces[739].y = 531;faces[739].z = 534;
faces[740].x = 531;faces[740].y = 535;faces[740].z = 534;
faces[741].x = 534;faces[741].y = 536;faces[741].z = 89;
faces[742].x = 83;faces[742].y = 525;faces[742].z = 529;
faces[743].x = 534;faces[743].y = 89;faces[743].z = 529;
faces[744].x = 239;faces[744].y = 243;faces[744].z = 712;
faces[745].x = 243;faces[745].y = 253;faces[745].z = 712;
faces[746].x = 253;faces[746].y = 716;faces[746].z = 712;
faces[747].x = 71;faces[747].y = 72;faces[747].z = 517;
faces[748].x = 72;faces[748].y = 73;faces[748].z = 516;
faces[749].x = 517;faces[749].y = 72;faces[749].z = 516;
faces[750].x = 73;faces[750].y = 74;faces[750].z = 514;
faces[751].x = 515;faces[751].y = 516;faces[751].z = 73;
faces[752].x = 723;faces[752].y = 291;faces[752].z = 708;
faces[753].x = 291;faces[753].y = 292;faces[753].z = 251;
faces[754].x = 708;faces[754].y = 291;faces[754].z = 251;
faces[755].x = 431;faces[755].y = 439;faces[755].z = 20;
faces[756].x = 2;faces[756].y = 428;faces[756].z = 431;
faces[757].x = 274;faces[757].y = 275;faces[757].z = 735;
faces[758].x = 13;faces[758].y = 14;faces[758].z = 444;
faces[759].x = 272;faces[759].y = 738;faces[759].z = 739;
faces[760].x = 694;faces[760].y = 223;faces[760].z = 204;
faces[761].x = 204;faces[761].y = 196;faces[761].z = 694;
faces[762].x = 196;faces[762].y = 641;faces[762].z = 694;
faces[763].x = 693;faces[763].y = 695;faces[763].z = 694;
faces[764].x = 580;faces[764].y = 116;faces[764].z = 117;
faces[765].x = 764;faces[765].y = 298;faces[765].z = 299;
faces[766].x = 758;faces[766].y = 760;faces[766].z = 299;
faces[767].x = 421;faces[767].y = 835;faces[767].z = 836;
faces[768].x = 142;faces[768].y = 614;faces[768].z = 615;
faces[769].x = 317;faces[769].y = 775;faces[769].z = 776;
faces[770].x = 771;faces[770].y = 321;faces[770].z = 322;
faces[771].x = 859;faces[771].y = 861;faces[771].z = 378;
faces[772].x = 119;faces[772].y = 577;faces[772].z = 582;
faces[773].x = 773;faces[773].y = 770;faces[773].z = 772;
faces[774].x = 351;faces[774].y = 791;faces[774].z = 792;
faces[775].x = 158;faces[775].y = 159;faces[775].z = 629;
faces[776].x = 159;faces[776].y = 160;faces[776].z = 628;
faces[777].x = 158;faces[777].y = 629;faces[777].z = 630;
faces[778].x = 161;faces[778].y = 162;faces[778].z = 627;
faces[779].x = 162;faces[779].y = 170;faces[779].z = 638;
faces[780].x = 157;faces[780].y = 158;faces[780].z = 630;
faces[781].x = 162;faces[781].y = 638;faces[781].z = 627;
faces[782].x = 629;faces[782].y = 159;faces[782].z = 628;
faces[783].x = 160;faces[783].y = 161;faces[783].z = 625;
faces[784].x = 627;faces[784].y = 625;faces[784].z = 161;
faces[785].x = 625;faces[785].y = 626;faces[785].z = 160;
faces[786].x = 64;faces[786].y = 504;faces[786].z = 505;
faces[787].x = 809;faces[787].y = 328;faces[787].z = 330;
faces[788].x = 330;faces[788].y = 353;faces[788].z = 838;
faces[789].x = 138;faces[789].y = 139;faces[789].z = 582;
faces[790].x = 422;faces[790].y = 423;faces[790].z = 833;
faces[791].x = 421;faces[791].y = 422;faces[791].z = 834;
faces[792].x = 847;faces[792].y = 355;faces[792].z = 367;
faces[793].x = 355;faces[793].y = 364;faces[793].z = 367;
faces[794].x = 178;faces[794].y = 179;faces[794].z = 639;
faces[795].x = 179;faces[795].y = 167;faces[795].z = 639;
faces[796].x = 167;faces[796].y = 623;faces[796].z = 639;
faces[797].x = 782;faces[797].y = 313;faces[797].z = 780;
faces[798].x = 313;faces[798].y = 314;faces[798].z = 780;
faces[799].x = 314;faces[799].y = 316;faces[799].z = 776;
faces[800].x = 887;faces[800].y = 888;faces[800].z = 886;
faces[801].x = 888;faces[801].y = 384;faces[801].z = 886;
faces[802].x = 456;faces[802].y = 457;faces[802].z = 30;
faces[803].x = 417;faces[803].y = 418;faces[803].z = 892;
faces[804].x = 860;faces[804].y = 856;faces[804].z = 858;
faces[805].x = 858;faces[805].y = 871;faces[805].z = 860;
faces[806].x = 871;faces[806].y = 866;faces[806].z = 862;
faces[807].x = 860;faces[807].y = 871;faces[807].z = 862;
faces[808].x = 866;faces[808].y = 867;faces[808].z = 864;
faces[809].x = 867;faces[809].y = 877;faces[809].z = 875;
faces[810].x = 877;faces[810].y = 878;faces[810].z = 884;
faces[811].x = 878;faces[811].y = 882;faces[811].z = 885;
faces[812].x = 884;faces[812].y = 878;faces[812].z = 885;
faces[813].x = 882;faces[813].y = 830;faces[813].z = 881;
faces[814].x = 830;faces[814].y = 825;faces[814].z = 889;
faces[815].x = 889;faces[815].y = 825;faces[815].z = 831;
faces[816].x = 825;faces[816].y = 819;faces[816].z = 831;
faces[817].x = 819;faces[817].y = 797;faces[817].z = 796;
faces[818].x = 831;faces[818].y = 819;faces[818].z = 824;
faces[819].x = 797;faces[819].y = 793;faces[819].z = 798;
faces[820].x = 793;faces[820].y = 790;faces[820].z = 795;
faces[821].x = 781;faces[821].y = 774;faces[821].z = 770;
faces[822].x = 774;faces[822].y = 772;faces[822].z = 770;
faces[823].x = 793;faces[823].y = 795;faces[823].z = 798;
faces[824].x = 790;faces[824].y = 781;faces[824].z = 784;
faces[825].x = 795;faces[825].y = 790;faces[825].z = 784;
faces[826].x = 867;faces[826].y = 875;faces[826].z = 864;
faces[827].x = 875;faces[827].y = 877;faces[827].z = 884;
faces[828].x = 781;faces[828].y = 770;faces[828].z = 767;
faces[829].x = 864;faces[829].y = 862;faces[829].z = 866;
faces[830].x = 781;faces[830].y = 767;faces[830].z = 784;
faces[831].x = 767;faces[831].y = 763;faces[831].z = 762;
faces[832].x = 778;faces[832].y = 773;faces[832].z = 726;
faces[833].x = 726;faces[833].y = 816;faces[833].z = 778;
faces[834].x = 816;faces[834].y = 721;faces[834].z = 731;
faces[835].x = 778;faces[835].y = 816;faces[835].z = 779;
faces[836].x = 721;faces[836].y = 722;faces[836].z = 717;
faces[837].x = 722;faces[837].y = 718;faces[837].z = 717;
faces[838].x = 718;faces[838].y = 707;faces[838].z = 720;
faces[839].x = 707;faces[839].y = 699;faces[839].z = 706;
faces[840].x = 720;faces[840].y = 707;faces[840].z = 706;
faces[841].x = 699;faces[841].y = 700;faces[841].z = 710;
faces[842].x = 700;faces[842].y = 702;faces[842].z = 711;
faces[843].x = 702;faces[843].y = 709;faces[843].z = 713;
faces[844].x = 709;faces[844].y = 715;faces[844].z = 713;
faces[845].x = 715;faces[845].y = 745;faces[845].z = 714;
faces[846].x = 745;faces[846].y = 748;faces[846].z = 742;
faces[847].x = 742;faces[847].y = 748;faces[847].z = 746;
faces[848].x = 748;faces[848].y = 751;faces[848].z = 753;
faces[849].x = 751;faces[849].y = 750;faces[849].z = 759;
faces[850].x = 753;faces[850].y = 751;faces[850].z = 759;
faces[851].x = 750;faces[851].y = 754;faces[851].z = 759;
faces[852].x = 754;faces[852].y = 761;faces[852].z = 763;
faces[853].x = 761;faces[853].y = 762;faces[853].z = 763;
faces[854].x = 762;faces[854].y = 783;faces[854].z = 767;
faces[855].x = 783;faces[855].y = 787;faces[855].z = 784;
faces[856].x = 787;faces[856].y = 807;faces[856].z = 813;
faces[857].x = 807;faces[857].y = 811;faces[857].z = 813;
faces[858].x = 811;faces[858].y = 837;faces[858].z = 815;
faces[859].x = 837;faces[859].y = 850;faces[859].z = 845;
faces[860].x = 850;faces[860].y = 849;faces[860].z = 845;
faces[861].x = 849;faces[861].y = 853;faces[861].z = 845;
faces[862].x = 845;faces[862].y = 815;faces[862].z = 837;
faces[863].x = 811;faces[863].y = 815;faces[863].z = 814;
faces[864].x = 754;faces[864].y = 763;faces[864].z = 759;
faces[865].x = 811;faces[865].y = 814;faces[865].z = 813;
faces[866].x = 787;faces[866].y = 813;faces[866].z = 784;
faces[867].x = 746;faces[867].y = 748;faces[867].z = 753;
faces[868].x = 700;faces[868].y = 711;faces[868].z = 710;
faces[869].x = 717;faces[869].y = 718;faces[869].z = 720;
faces[870].x = 779;faces[870].y = 816;faces[870].z = 731;
faces[871].x = 783;faces[871].y = 784;faces[871].z = 767;
faces[872].x = 717;faces[872].y = 731;faces[872].z = 721;
faces[873].x = 881;faces[873].y = 885;faces[873].z = 882;
faces[874].x = 710;faces[874].y = 706;faces[874].z = 699;
faces[875].x = 889;faces[875].y = 881;faces[875].z = 830;
faces[876].x = 713;faces[876].y = 711;faces[876].z = 702;
faces[877].x = 824;faces[877].y = 819;faces[877].z = 796;
faces[878].x = 798;faces[878].y = 796;faces[878].z = 797;
faces[879].x = 714;faces[879].y = 713;faces[879].z = 715;
faces[880].x = 189;faces[880].y = 655;faces[880].z = 656;
faces[881].x = 122;faces[881].y = 123;faces[881].z = 593;
faces[882].x = 123;faces[882].y = 124;faces[882].z = 592;
faces[883].x = 124;faces[883].y = 129;faces[883].z = 596;
faces[884].x = 592;faces[884].y = 593;faces[884].z = 123;
faces[885].x = 593;faces[885].y = 602;faces[885].z = 122;
faces[886].x = 124;faces[886].y = 596;faces[886].z = 592;
faces[887].x = 579;faces[887].y = 117;faces[887].z = 122;
faces[888].x = 694;faces[888].y = 696;faces[888].z = 222;
faces[889].x = 858;faces[889].y = 856;faces[889].z = 853;
faces[890].x = 856;faces[890].y = 854;faces[890].z = 853;
faces[891].x = 818;faces[891].y = 425;faces[891].z = 426;



    }
    void Scale(float scale)
    {
        for (int t=0; t<N_VERT_Logo3DNSP; ++t) {
          vert[t].x*=scale;
          vert[t].y*=scale;
          vert[t].z*=scale;
        }
    }
	
};



String degToCompass16(float gradi)
{
    int val=int((gradi/22.5)+.5);
    String arr[16]={"N","NNE","NE","ENE","E","ESE", "SE", "SSE","S","SSW","SW","WSW","W","WNW","NW","NNW"};
    return arr[(val % 16)];
}

String degToCompass8(float gradi)
{
    int val=int((gradi/45.0)+.5);
    String arr[16]={"N","NE","E", "SE","S","SW","W","NW"};
    return arr[(val % 8)];
}



class UltraDistSensor {
	private:
		uint8_t trig,echo;
		bool common;
		uint16_t timeOut;
		int16_t air_temperature; //celsius
		unsigned long soundSpeed;
		unsigned long getReading()
		{
			if(common==true) pinMode(trig,OUTPUT);
			digitalWrite(trig, LOW);
			delayMicroseconds(2);
			digitalWrite(trig, HIGH);
			delayMicroseconds(10);
			digitalWrite(trig, LOW);
			if(common==true) pinMode(echo, INPUT);
			return pulseIn(echo, HIGH,timeOut);
		}
	public:
		UltraDistSensor(){};
		void attach(uint8_t TrigerPin,uint8_t EchoPin, int16_t air_temperature_, uint16_t timeout=20000ul)
		{
			this->air_temperature = air_temperature_; //celsius
			this->soundSpeed = 331300+606*this->air_temperature; // mm/s;
			trig=TrigerPin;
			echo=EchoPin;
			timeOut=timeout;
			common= trig==echo ? true:false;
			pinMode(trig, OUTPUT);
			pinMode(echo, INPUT);
		}
		unsigned long distanceInmm()
		{
			unsigned long distance = this->soundSpeed/100; //speed is in mm/s, duration in microseconds: 6 zeroes need to be removed, 2 go out here, the baÃ±ance later
			distance = distance*getReading(); // distance (mm) = time (total sound travelling distance, microseconds) * speed (mm/s)
			distance = distance/20000;// I remove the remaing 4 zeros here, plus the effect of two-way travel of the sound; this care with the zeros is because of size limitations of long 
			return distance;
		}
		void changeTimeout(uint16_t timeout)
		{
			timeOut=timeout;
		}
		void changeAirTemperature(int16_t air_temperature_)
		{
			this->air_temperature = air_temperature_; //celsius
			this->soundSpeed = 331300+606*this->air_temperature; // mm/s;
		}
		
};


#endif