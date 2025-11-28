/*
 * Copyright (C) 2025 Global Graphics Software Ltd. All rights reserved.
 */

package simpleapexrender;

public class StopWatch
{
    public StopWatch(String nm)
    {
        m_label = nm;
        m_delta = 0.0;
        m_totalCount = 0;
    }

    public void start ()
    {
        m_startTime = System.nanoTime();
    }

    public void end (int totalCount)
    {
        long endTime = System.nanoTime();
        long timeSpan = endTime - m_startTime;

        m_delta = timeSpan / 1e9;
        m_totalCount = totalCount;
    }

    public void printTimeDiff ()
    {
        System.out.printf("ElapsedTime::%s %f\n", m_label, m_delta);
    }

    public void printSummary ()
    {
        double avg = (1.0 * m_delta) / m_totalCount;
        double tput = 1.0 / avg;

        System.out.printf("ElapsedTime::%s::AVG  %f\n", m_label, avg);
        System.out.printf("ElapsedTime::%s::TPUT %f\n", m_label, tput);
    }

    double getElapsedTime()
    {
        return m_delta;
    }

    long            m_startTime;
    public double   m_delta;

    String          m_label;
    public int      m_totalCount;
}